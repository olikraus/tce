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

int strset(char **s, const char *t)
{
	if ( s == NULL )
		return 0;
	if ( *s != NULL )
		free(*s);
	*s = NULL;
	
	if ( t != NULL )
	{
		*s = strdup(t);
		if ( *s == NULL )
			return 0;
	}
	return 1;
}

/* 
	check whether the line a0, a1 intersecs with v0, v1

	precondition 
		1) a0 <= a1
		2) b0 <= b1
*/

static int is_intersection(long a0, long a1, long b0, long b1) 
{
	if ( a1 < b0 )
		return 0;
	if ( a0 > b1 )
		return 0;
	return 1;
}

int is_rectangle_intersection(const rect_t *a, const rect_t *b)
{
	if ( is_intersection(a->x0, a->x1, b->x0, b->x1) == 0 )
		return 0;
	if ( is_intersection(a->y0, a->y1, b->y0, b->y1) == 0 )
		return 0;
	return 1;
}

void clear_rectangle(rect_t *r)
{
	r->x0 = 0;
	r->x1 = 0;
	r->y0 = 0;
	r->y1 = 0;
}

void invalid_rectangle(rect_t *r)
{
	r->x0 = 1;
	r->x1 = 0;
	r->y0 = 1;
	r->y1 = 0;
}

void expand_rectangle(rect_t *to_be_expanded, const rect_t *r)
{
	if ( to_be_expanded->x0 > r->x0 )
		to_be_expanded->x0 = r->x0;
	if ( to_be_expanded->x1 < r->x1 )
		to_be_expanded->x1 = r->x1;
	if ( to_be_expanded->y0 > r->y0 )
		to_be_expanded->y0 = r->y0;
	if ( to_be_expanded->y1 < r->y1 )
		to_be_expanded->y1 = r->y1;	
}



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

tig_t *tig_Open(const char *name)
{
	tig_t *tig;
	tig = (tig_t *)malloc(sizeof(tig_t));
	if ( tig != NULL )
	{
		tig->name = NULL;
		
		if ( strset(&(tig->name), "<none>") != 0 )
		{
			clear_rectangle(&(tig->area));
			clear_rectangle(&(tig->move_start_area));
			tig->is_selected = 0;
			
			return tig;
		}
		free(tig);
	}
	return NULL;
}

void tig_Close(tig_t *tig)
{
	strset(&(tig->name), NULL);
	free(tig);
}

int tig_SetName(tig_t *tig, const char *s)
{
	return strset(&(tig->name), s);
}

long tig_GetWidth(tig_t *tig)
{
	return tig->area.x1 - tig->area.x0 + 1;
}

long tig_GetHeight(tig_t *tig)
{
	return tig->area.y1 - tig->area.y0 + 1;
}

/* replaced by tcg_IsCatched() */
/*
int tig_IsInside(tig_t *tig, long x, long y)
{
	if ( x < tig->x )
		return 0;
	if ( y < tig->y )
		return 0;
	if ( x > tig->x + tig_GetWidth(tig) )
		return 0;
	if ( y > tig->y + tig_GetHeight(tig) )
		return 0;
	return 1;	
}
*/


void tig_Select(tig_t *tig)
{
	if ( tig->is_selected !=0 )
		return;	/* already selected */
	
	tig->is_selected = 1;
}

void tig_Deselect(tig_t *tig)
{
	if ( tig->is_selected == 0 )
		return;	/* not selected */
	
	tig->is_selected = 0;
}

void tig_StartMove(tig_t *tig)
{
	if ( tig->is_selected != 0 )
		tig->move_start_area = tig->area;
}

void tig_ApplyMove(tig_t *tig, long delta_x, long delta_y)
{
	if ( tig->is_selected != 0 )
	{
		tig->area.x0 = tig->move_start_area.x0 + delta_x;
		tig->area.x1 = tig->move_start_area.x1 + delta_x;
		tig->area.y0 = tig->move_start_area.y0 + delta_y;
		tig->area.y1 = tig->move_start_area.y1 + delta_y;		
	}
}


void tig_AbortMove(tig_t *tig)
{
	if ( tig->is_selected != 0 )
		tig->area = tig->move_start_area;
}

void tig_FinishMove(tig_t *tig)
{
	 /* nothing to do */
}

/*========================================*/

aig_t *aig_Open(void)
{
	int i;
	aig_t *aig;
	aig = (aig_t *)malloc(sizeof(aig_t));
	if ( aig != NULL )
	{
		aig->eig_src = -1;
		aig->dir_src = 0;
		aig->eig_dest = -1;
		aig->dir_dest = 0;

		aig->dfv_cnt = 0;
		for( i = 0; i <  AIG_DFV_MAX; i++ )
		{
			aig->dfv_list[i].v = 0;
			aig->dfv_list[i].min = 0;
			aig->dfv_list[i].max = 0;
		}

		aig->point_val_cnt = 0;
		for( i = 0; i <  AIG_POINT_MAX*2; i++ )
		{
			aig->point_val_list[i] = 0;
			aig->dfv_ref_list[i] = -1;
		}		
		return aig;
	}
	return NULL;
}

void aig_Close(aig_t *aig)
{
	free(aig);
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
					tgc_CalculateDimension(tcg);
					invalid_rectangle(&(tcg->catch_area));
					tcg->state = TCG_STATE_IDLE;
					return tcg;
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

void tcg_Close(tcg_t *tcg)
{
	tcg_ClearTigList(tcg);
	tcg_ClearAigList(tcg);
	ps_Close(tcg->aig_list);
	ps_Close(tcg->tig_list);
	tcgv_Close(tcg->tcgv);
	free(tcg);
}

/* returns position or -1 */
static int tcg_AddPlainTig(tcg_t *tcg)
{
	tig_t *tig;
	int idx;
	tig = tig_Open(NULL);
	if ( tig != NULL )
	{
		idx = ps_Add(tcg->tig_list, tig);
		if ( idx >= 0 )
		{
			return idx;
		}
		tig_Close(tig);
	}
	return -1;
}

/* returns position or -1 */
static int tcg_AddPlainAig(tcg_t *tcg)
{
	aig_t *aig;
	int idx;
	aig = aig_Open();
	if ( aig != NULL )
	{
		idx = ps_Add(tcg->aig_list, aig);
		if ( idx >= 0 )
		{
			return idx;
		}
		aig_Close(aig);
	}
	return -1;
}

void tcg_DeleteTig(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return;
	tig = tcg_GetTig(tcg, idx);
	if ( tig == NULL )
		return;
	tig_Close(tig);
	ps_Del(tcg->tig_list, idx);
}

int tcg_AddTig(tcg_t *tcg, const char *name, long x, long y)
{
	int idx;
	tig_t *tig;
	idx = tcg_AddPlainTig(tcg);
	if ( idx >= 0 )
	{
		tig = tcg_GetTig(tcg, idx);
		if ( tig_SetName(tig, name) != 0 )
		{
			tig->area.x0 = x;
			tig->area.y0 = y;
			tig->area.x1 = x + 60;
			tig->area.y1 = y + 20;
			tgc_CalculateDimension(tcg);
			return idx;
		}
		tcg_DeleteTig(tcg, idx);
	}
	return -1;
}


int tcg_AddAig(tcg_t *tcg, int eig_src, int dir_src, int eig_dest, int dir_dest)
{
	int idx;
	aig_t *aig;
	idx = tcg_AddPlainAig(tcg);
	if ( idx >= 0 )
	{
		aig = tcg_GetAig(tcg, idx);
		aig->eig_src = eig_src;
		aig->dir_src = dir_src;
		aig->eig_dest = eig_dest;
		aig->dir_dest = dir_dest;
		tcg_CalculateAigPath(tcg, idx);
		tcg_ShowAigPoints(tcg, idx);

		return idx;
	}
	return -1;
}


int tcg_IsSelected(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return 0;
	tig = tcg_GetTig(tcg, idx);
	return tig->is_selected;
}

void tcg_Select(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return;
	tig = tcg_GetTig(tcg, idx);
	tig_Select(tig);
}

void tcg_Deselect(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return;
	tig = tcg_GetTig(tcg, idx);
	tig_Deselect(tig);
}

/* this will deselect everything  */
void tcg_DeselectAll(tcg_t *tcg)
{
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tcg_Deselect(tcg, i);
	}
}


/* check if the element with "idx" is catched */
int tcg_IsCatched(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return 0;
	tig = tcg_GetTig(tcg, idx);
	if ( tig == NULL )
		return 0;
	return is_rectangle_intersection(&(tig->area), &(tcg->catch_area));

}


void tcg_AddCatchedToSelection(tcg_t *tcg)
{
	tig_t *tig;
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		if ( tcg_IsCatched(tcg, i) != 0 )
		{
			tig = tcg_GetTig(tcg, i);
			tig_Select(tig);
		}
	}
}


int tcg_GetElementOverPosition(tcg_t *tcg, long x, long y)
{
	tig_t *tig;
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( (tig->area.x0 <= x && x <= tig->area.x1) && (tig->area.y0 <= y && y <= tig->area.y1) )
		{
			return i;
		}
	}
	return -1;
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
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( tig->is_selected != 0 )
			tig_StartMove(tig);
	}
	// puts("tcg_StartMove");
}

void tcg_ApplyMove(tcg_t *tcg, long delta_x, long delta_y)
{
	tig_t *tig;
	int i;
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tig = tcg_GetTig(tcg, i);
		if ( tig->is_selected != 0 )
			tig_ApplyMove(tig, delta_x, delta_y);
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
	int idx;
	switch(event)
	{
		case TCG_EVENT_BUTTON_DOWN:
			idx = tcg_GetElementOverPosition(tcg, x, y);
			if ( idx >= 0 )
			{
				/* user has done a button press on an element */
				tcg_SetCatchAreaToPoint(tcg, x, y);
				/* store the reference position */
				tcg->start_x = x;				
				tcg->start_y = y;
				if ( tcg_IsSelected(tcg, idx) != 0 )
				{
					tcg->state = TCG_STATE_SELECTON_MOVE;
				}
				else
				{
					tcg_DeselectAll(tcg);
					tcg_Select(tcg, idx);
					tcg->state = TCG_STATE_SINGLE_MOVE;
				}				
				r = 1;	/* catch area change */
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
			idx = tcg_GetElementOverPosition(tcg, x, y);
			if ( idx >= 0 )
			{
				/* shift mouse button press: single element (de)selection */
				/* state does not change here, change is applied directly */
				tcg_SetCatchAreaToPoint(tcg, x, y);
				if ( tcg_IsSelected(tcg, idx) != 0 )
				{
					tcg_Deselect(tcg, idx);
				}
				else
				{
					tcg_Select(tcg, idx);					
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
	int idx;
	switch(event)
	{
		case TCG_EVENT_SHIFT_BUTTON_DOWN:
		case TCG_EVENT_BUTTON_DOWN:
			/* does not make sense here, button is already down */
			r = 0;	
			break;
		case TCG_EVENT_MOUSE_MOVE:
			tcg_SetCatchAreaToPoint(tcg, x, y);
		
			idx = tcg_GetElementOverPosition(tcg, tcg->start_x, tcg->start_y);
			if ( idx < 0 )
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



