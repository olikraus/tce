/*

	tcg.c
	
	Tool Chain Editor, graph procedures
	Copyright (C) 2016 olikraus@gmail.com

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
*/

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "tcg.h"


/*========================================*/

tcgv_t *tcgv_Open(tcg_t *tcg)
{
	tcgv_t *tcgv;
	tcgv = (tcgv_t *)malloc(sizeof(tcgv_t));
	if ( tcgv != NULL )
	{
		tcgv->zoom = 1.0;
		tcgv->x = 0;
		tcgv->y = 0;
		return tcgv;
	}
	return NULL;
}

void tcgv_Close(tcgv_t *tcgv)
{
	free(tcgv);
}





/*========================================*/

void tgc_CalculateDimension(tcg_t *tcg)
{
	int is_first;
	int i;
	tig_t *tig;
	i = -1;
	is_first = 1;
	
	clear_rectangle(&(tcg->graph_dimension));
	
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( is_first != 0 )
		{
			tcg->graph_dimension = tig->area;
			is_first = 0;
		}
		else
		{
			expand_rectangle(&(tcg->graph_dimension), &(tig->area));
		}
	}
}

long tgc_GetGraphXFromView(tcg_t *tcg, double x)
{
	return x/tcg->tcgv->zoom + tcg->tcgv->x;
}

long tgc_GetGraphYFromView(tcg_t *tcg, double y)
{
	return y/tcg->tcgv->zoom + tcg->tcgv->y;
}

double tgc_GetViewXFromGraph(tcg_t *tcg, long x)
{
	return (x - tcg->tcgv->x)*tcg->tcgv->zoom;
}

double tgc_GetViewYFromGraph(tcg_t *tcg, long y)
{
	return (y - tcg->tcgv->y)*tcg->tcgv->zoom;
}

tcg_t *tcg_Open(void)
{
	tcg_t *tcg;
	tcg = (tcg_t *)malloc(sizeof(tcg_t));
	if ( tcg != NULL )
	{
		tcg->tcgv = tcgv_Open(tcg);
		if ( tcg->tcgv != NULL )
		{
			tcg->tig_list = ps_Open();
			if ( tcg->tig_list != NULL )
			{
				tcg->aig_list = ps_Open();
				if ( tcg->aig_list != NULL )
				{
					tcg->seg_list = ps_Open();
					if ( tcg->seg_list != NULL )
					{
						tgc_CalculateDimension(tcg);
						invalid_rectangle(&(tcg->catch_area));
						tcg->state = TCG_STATE_IDLE;
						return tcg;
					}
					ps_Close(tcg->aig_list);
				}
				ps_Close(tcg->tig_list);
			}
			tcgv_Close(tcg->tcgv);
		}
		free(tcg);
	}
	return NULL;
}

static void tcg_ClearTigList(tcg_t *tcg)
{
	int i;
	tig_t *tig;
	
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		tig_Close(tig);
		tcg_SetTig(tcg, i, NULL);
	}
	ps_Clear(tcg->tig_list);
}

static void tcg_ClearAigList(tcg_t *tcg)
{
	int i;
	aig_t *aig;
	
	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		aig = tcg_GetAig(tcg, i);
		aig_Close(aig);
		tcg_SetAig(tcg, i, NULL);
	}
	ps_Clear(tcg->tig_list);
}

static void tcg_ClearSegList(tcg_t *tcg)
{
	int i;
	seg_t *seg;
	
	i = -1;
	while( tcg_WhileSeg(tcg, &i) )
	{
		seg = tcg_GetSeg(tcg, i);
		seg_Close(seg);
		tcg_SetSeg(tcg, i, NULL);
	}
	ps_Clear(tcg->seg_list);
}

void tcg_Close(tcg_t *tcg)
{
	tcg_ClearTigList(tcg);
	tcg_ClearAigList(tcg);
	tcg_ClearSegList(tcg);
	ps_Close(tcg->aig_list);
	ps_Close(tcg->tig_list);
	ps_Close(tcg->seg_list);
	tcgv_Close(tcg->tcgv);
	free(tcg);
}




/* this will deselect everything  */
void tcg_DeselectAll(tcg_t *tcg)
{
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tcg_DeselectTig(tcg, i);
	}
	
	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		tcg_DeselectAig(tcg, i);
	}
}



void tcg_AddCatchedToSelection(tcg_t *tcg)
{
	//tig_t *tig;
	int i, j, cnt;
	

	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		if ( tcg_IsTigCatched(tcg, i) != 0 )
		{
			tcg_SelectTig(tcg, i);
		}
	}
	
	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		cnt = tcg_GetAigSegCnt(tcg, i);
		for ( j = 0; j < cnt; j++ )
		{
			if ( tcg_IsAigSegCatched(tcg, i, j) != 0 )
			{
				tcg_SelectAigSeg(tcg, i, j);
			}
		}
	}
	
}

/*
	Check if x/y is above tig or aig segment.
	Store the result in 
		tcg->tig_idx
		tcg->aig_idx 
		tcg->seg_pos
	return 0 if there is no element (tig or segment) at x/y.
*/
int tcg_GetElementOverPosition(tcg_t *tcg, long x, long y)
{
	tig_t *tig;
	rect_t r;
	int i, j, cnt;
	
	tcg->tig_idx = -1;
	tcg->aig_idx = -1;
	tcg->seg_pos = -1;
	tcg->con_dir = -1;
	tcg->con_pos = -1;
	
	
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( (tig->area.x0 <= x && x <= tig->area.x1) && (tig->area.y0 <= y && y <= tig->area.y1) )
		{
			tcg->tig_idx = i;
			
			tcg_GetCatchedConnect(tcg, i, &(tcg->con_dir), &(tcg->con_pos));
			return 1;
		}
	}
	
	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		cnt = tcg_GetAigSegCnt(tcg, i)-1;
		for ( j = 1; j < cnt; j++ )
		{
			tcg_GetAigSegRect(tcg, i, j, &r);
			if ( (r.x0 <= x && x <= r.x1) && (r.y0 <= y && y <= r.y1) )
			{
				tcg->aig_idx = i;
				tcg->seg_pos = j;
				return 1;
			}
		}
	}
	
	return 0;
}

int tcg_IsElementSelected(tcg_t *tcg)
{
	if ( tcg->tig_idx >= 0 )
		return tcg_IsTigSelected(tcg, tcg->tig_idx);
	if ( tcg->aig_idx >= 0 )
		return tcg_IsAigSegSelected(tcg, tcg->aig_idx, tcg->seg_pos);
	return 0;
}

void tcg_SelectElement(tcg_t *tcg)
{
	if ( tcg->tig_idx >= 0 )
		tcg_SelectTig(tcg, tcg->tig_idx);
	if ( tcg->aig_idx >= 0 )
		tcg_SelectAigSeg(tcg, tcg->aig_idx, tcg->seg_pos);
}

void tcg_DeselectElement(tcg_t *tcg)
{
	if ( tcg->tig_idx >= 0 )
		tcg_DeselectTig(tcg, tcg->tig_idx);
	if ( tcg->aig_idx >= 0 )
		tcg_DeselectAigSeg(tcg, tcg->aig_idx, tcg->seg_pos);
}


/*========================================*/
/*
	tce high level user interaction

	Inputs are:
		Event
			- mouse button press/down (D)
			- mouse button press/down with shift (SD)
			- mouse move (M)
			- mouse button release/up (U)
		Position (comes with all events)
*/

void tcg_SetCatchAreaToPoint(tcg_t *tcg, long x, long y)
{
	tcg->catch_area.x0 = x;
	tcg->catch_area.x1 = x;
	tcg->catch_area.y0 = y;
	tcg->catch_area.y1 = y;	
}

void tcg_SetCatchAreaFromStartToPoint(tcg_t *tcg, long x, long y)
{
	if ( tcg->start_x < x )
	{
		tcg->catch_area.x0 = tcg->start_x;
		tcg->catch_area.x1 = x;
	}
	else
	{
		tcg->catch_area.x0 = x;
		tcg->catch_area.x1 = tcg->start_x;
	}
	if ( tcg->start_y < y )
	{
		tcg->catch_area.y0 = tcg->start_y;
		tcg->catch_area.y1 = y;
	}
	else
	{
		tcg->catch_area.y0 = y;
		tcg->catch_area.y1 = tcg->start_y;
	}
	// printf("catch area: x0=%ld x1=%ld y0=%ld y1=%ld\n", tcg->catch_area.x0, tcg->catch_area.x1, tcg->catch_area.y0, tcg->catch_area.y1);
}


void tcg_StartMove(tcg_t *tcg)
{
	tig_t *tig;
	int i, j, cnt;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( tig->is_selected != 0 )
			tig_StartMove(tig);
	}
	
	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		cnt = tcg_GetAigSegCnt(tcg, i);
		for ( j = 0; j < cnt; j++ )
		{
			if ( tcg_IsAigSegSelected(tcg, i, j) != 0 )
			{
				tcg_StartAigSegMove(tcg, i, j);
			}
		}
	}
	// puts("tcg_StartMove");
}

void tcg_ApplyMove(tcg_t *tcg, long delta_x, long delta_y)
{
	tig_t *tig;
	int i, j, cnt;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( tig->is_selected != 0 )
			tig_ApplyMove(tig, delta_x, delta_y);
	}
	

	i = -1;
	while( tcg_WhileAig(tcg, &i) )
	{
		cnt = tcg_GetAigSegCnt(tcg, i);
		for ( j = 0; j < cnt; j++ )
		{
			if ( tcg_IsAigSegSelected(tcg, i, j) != 0 )
			{
				tcg_ApplyAigSegMove(tcg, i, j, delta_x, delta_y);
			}
		}
	}
	
	// printf("tcg_ApplyMove %ld %ld\n", delta_x, delta_y);
}

void tcg_FinalizeMove(tcg_t *tcg)
{
	/* finalizes the move operation */
}


static int tcg_handle_state_idle(tcg_t *tcg, int event , long x, long y)
{
	int r = 0;
	switch(event)
	{
		case TCG_EVENT_BUTTON_DOWN:
			if ( tcg_GetElementOverPosition(tcg, x, y) )
			{
				if ( tcg->con_dir >= 0 && tcg->con_pos >= 0 )
				{
					r = 1;	/* catch area change */
					puts("Connector pressed");
					/* store the reference position */
					tcg->start_x = x;				
					tcg->start_y = y;
					/* start a new path at the connector */
					tcg->path_aig_idx = tcg_StartNewAigPath(tcg, tcg->tig_idx, tcg->con_dir, tcg->con_pos);
					
					/* continue with the state */
					tcg->state = TCG_STATE_DO_PATH;
					puts("TCG_STATE_DO_PATH");
				}
				else
				{
					/* user has done a button press on an element */
					tcg_SetCatchAreaToPoint(tcg, x, y);
					/* store the reference position */
					tcg->start_x = x;				
					tcg->start_y = y;
					if ( tcg_IsElementSelected(tcg) != 0 )
					{
						puts("TCG_STATE_SELECTON_MOVE");
						tcg->state = TCG_STATE_SELECTON_MOVE;
					}
					else
					{
						tcg_DeselectAll(tcg);
						tcg_SelectElement(tcg);
						puts("TCG_STATE_SINGLE_MOVE");
						tcg->state = TCG_STATE_SINGLE_MOVE;
					}				
					r = 1;	/* catch area change */
				}
			}
			else
			{
				/* empty space selected, we might start the use of a catch area rectangle as selection */
				tcg_SetCatchAreaToPoint(tcg, x, y);
				tcg->start_x = x;
				tcg->start_y = y;
				tcg_DeselectAll(tcg);	
				tcg->state = TCG_STATE_REPLACE_CATCH_AREA;
				r = 1;	/* catch area change and deselection */
			}
			break;
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
			if ( tcg_GetElementOverPosition(tcg, x, y) )
			{
				/* shift mouse button press: single element (de)selection */
				/* state does not change here, change is applied directly */
				tcg_SetCatchAreaToPoint(tcg, x, y);
				if ( tcg_IsElementSelected(tcg) != 0 )
				{
					tcg_DeselectElement(tcg);
				}
				else
				{
					tcg_SelectElement(tcg);
				}
				r = 1;	/* catch area change and selection change of elemnt(idx) */
			}
			else
			{
				/* empty space selected, we might start an update of the selectoin with a catch area rectangle */
				tcg_SetCatchAreaToPoint(tcg, x, y);
				tcg->start_x = x;
				tcg->start_y = y;
				tcg->state = TCG_STATE_ADD_CATCH_AREA;
				r = 1;	/* catch area change */
			}
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			r = 1;
			break;			
		case TCG_EVENT_BUTTON_UP:
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_add_catch_area(tcg_t *tcg, int event , long x, long y)
{
	int r;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaFromStartToPoint(tcg, x, y);
			tcg->state = TCG_STATE_DO_CATCH_AREA_SELECTION;
			r = 1;	/* catch area change */
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			/* user changed his mind, just return to idle state */
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change */			
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_replace_catch_area(tcg_t *tcg, int event , long x, long y)
{
	int r;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			/* deselection already has happend */
			tcg_SetCatchAreaFromStartToPoint(tcg, x, y);
			tcg->state = TCG_STATE_DO_CATCH_AREA_SELECTION;
			r = 1;	/* catch area change */
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			/* user changed his mind, just return to idle state */
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change */			
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_do_catch_area_selection(tcg_t *tcg, int event , long x, long y)
{
	int r;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaFromStartToPoint(tcg, x, y);
			r = 1;	/* catch area change */				
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_AddCatchedToSelection(tcg);
			tcg_SetCatchAreaToPoint(tcg, x, y);
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change and selection state*/
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_single_move(tcg_t *tcg, int event , long x, long y)
{
	int r;
	//int idx;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaToPoint(tcg, x, y);
		
			//idx = tcg_GetElementOverPosition(tcg, tcg->start_x, tcg->start_y);
			if ( tcg_GetElementOverPosition(tcg, tcg->start_x, tcg->start_y) == 0 )
			{
				/* something is wrong, this shold not happen */
				tcg->state = TCG_STATE_IDLE;
				r = 1;	/* catch area change and deselection */
			}
			else
			{		
				tcg_StartMove(tcg);
				tcg_ApplyMove(tcg, x - tcg->start_x, y - tcg->start_y);
				tcg->state = TCG_STATE_DO_MOVEMENT;
				r = 1;	/* catch area change and moving state for selected elements */
			}
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			/* user changed his mind, just return to idle state */
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change */			
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_selection_move(tcg_t *tcg, int event , long x, long y)
{
	int r;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_StartMove(tcg);
			tcg_SetCatchAreaToPoint(tcg, x, y);
			tcg_ApplyMove(tcg, x - tcg->start_x, y - tcg->start_y);
			tcg->state = TCG_STATE_DO_MOVEMENT;
			r = 1;	/* catch area change and moving state for selected elements */			
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			/* user changed his mind, just return to idle state */
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change */			
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_do_movement(tcg_t *tcg, int event , long x, long y)
{
	int r;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			tcg_ApplyMove(tcg, x - tcg->start_x, y - tcg->start_y);
			tcg->state = TCG_STATE_DO_MOVEMENT;
			r = 1;	/* catch area change and moving state for selected elements */			
			break;
		case TCG_EVENT_BUTTON_UP:
			tcg_SetCatchAreaToPoint(tcg, x, y);
			/* tcg_ApplyMove(tcg, x, y); leave it at the previous pos */

			tcg_FinalizeMove(tcg);
			tcg->state = TCG_STATE_IDLE;
			r = 1;	/* catch area change and moving state for selected elements */
			break;
		default:
			r = 0;	/* nothing changed */
			break;
	}
	return r;
}

static int tcg_handle_state_do_path(tcg_t *tcg, int event , long x, long y)
{
	int r;
	
	if ( tcg->path_aig_idx < 0 )
	{
		tcg->state = TCG_STATE_IDLE;
	}
	else
	{
		switch(event)
		{
			case TCG_EVENT_SHIFT_BUTTON_DOWN:
			case TCG_EVENT_BUTTON_DOWN:
				/* does not make sense here, button is already down */
				r = 0;	
				break;
			case TCG_EVENT_MOUSE_MOVE:
			case TCG_EVENT_BUTTON_UP:
					/* store the reference position */
					tcg->start_x = x;				
					tcg->start_y = y;
			
				tcg_SetCatchAreaToPoint(tcg, x, y);
				/* tcg_ApplyMove(tcg, x, y); leave it at the previous pos */

				r = 1;	/* catch area change and moving state for selected elements */
				break;
			default:
				r = 0;	/* nothing changed */
				break;
		}
	}
	return r;
}

/*
	Event: one of
		TCG_EVENT_BUTTON_DOWN
		TCG_EVENT_SHIFT_BUTTON_DOWN
		TCG_EVENT_MOUSE_MOVE
		TCG_EVENT_BUTTON_UP
		
	return:
		0: 	no redraw required
		1:	redraw all
		2:	redraw of tcg_GetElementOverPosition(tcg, x, y)

	
*/
int tcg_SendEventWithGraphPosition(tcg_t *tcg, int event, long x, long y)
{
	int r;
	switch(tcg->state)
	{
		case TCG_STATE_IDLE: 
			r = tcg_handle_state_idle(tcg, event, x, y);
			break;
		case TCG_STATE_ADD_CATCH_AREA:
			r = tcg_handle_state_add_catch_area(tcg, event, x, y);
			break;
		case TCG_STATE_REPLACE_CATCH_AREA:
			r = tcg_handle_state_replace_catch_area(tcg, event, x, y);
			break;
		case TCG_STATE_DO_CATCH_AREA_SELECTION:
			r = tcg_handle_state_do_catch_area_selection(tcg, event, x, y);
			break;
		case TCG_STATE_SINGLE_MOVE:
			r = tcg_handle_state_single_move(tcg, event, x, y);
			break;			
		case TCG_STATE_SELECTON_MOVE:
			r = tcg_handle_state_selection_move(tcg, event, x, y);
			break;			
		case TCG_STATE_DO_MOVEMENT:
			r = tcg_handle_state_do_movement(tcg, event, x, y);
			break;
		case TCG_STATE_DO_PATH:
			r = tcg_handle_state_do_path(tcg, event, x, y);
			break;
			
		default:
			tcg->state = TCG_STATE_IDLE;
			r = 0;
			break;
	}
	return r;
}

int tcg_SendEventWithViewPosition(tcg_t *tcg, int event, double x, double y)
{
	return tcg_SendEventWithGraphPosition(tcg, event, 
		tgc_GetGraphXFromView(tcg, x), tgc_GetGraphYFromView(tcg, y) );
}



