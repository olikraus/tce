/*

	tcg_tig.c
	
	Tool Chain Editor, tool in graph procedures
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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "tcg.h"

/*========================================*/

tig_t *tig_Open(const char *name)
{
	tig_t *tig;
	tig = (tig_t *)malloc(sizeof(tig_t));
	if ( tig != NULL )
	{
		tig->name = NULL;
		
		if ( tce_strset(&(tig->name), "<none>") != 0 )
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
	tce_strset(&(tig->name), NULL);
	free(tig);
}

int tig_SetName(tig_t *tig, const char *s)
{
	return tce_strset(&(tig->name), s);
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


/* returns position or -1 */
int tcg_AddPlainTig(tcg_t *tcg)
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
			tig->area.x1 = x + TCG_CONNECT_GRID_SIZE*12;
			tig->area.y1 = y + TCG_CONNECT_GRID_SIZE*5;
			tgc_CalculateDimension(tcg);
			return idx;
		}
		tcg_DeleteTig(tcg, idx);
	}
	return -1;
}

int tcg_IsTigSelected(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return 0;
	tig = tcg_GetTig(tcg, idx);
	return tig->is_selected;
}

void tcg_SelectTig(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return;
	tig = tcg_GetTig(tcg, idx);
	tig_Select(tig);
}

void tcg_DeselectTig(tcg_t *tcg, int idx)
{
	tig_t *tig;
	if ( idx < 0 )
		return;
	tig = tcg_GetTig(tcg, idx);
	tig_Deselect(tig);
}

