/*

	tcg_aig.c
	
	Tool Chain Editor, artefact in graph procedures
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

aig_t *aig_Open(void)
{
	int i;
	aig_t *aig;
	aig = (aig_t *)malloc(sizeof(aig_t));
	if ( aig != NULL )
	{
		aig->tig_src = -1;
		aig->dir_src = 0;
		aig->tig_dest = -1;
		aig->dir_dest = 0;

		aig->dfv_cnt = 0;
		for( i = 0; i <  AIG_DFV_MAX; i++ )
		{
			aig->dfv_list[i].v = 0;
			aig->dfv_list[i].is_vertical = 0;
			aig->dfv_list[i].min = 0;
			aig->dfv_list[i].max = 0;
		}

		/*
		aig->point_val_cnt = 0;
		for( i = 0; i <  AIG_POINT_MAX*2; i++ )
		{
			aig->point_val_list[i] = 0;
			aig->dfv_ref_list[i] = -1;
		}
		*/		
		return aig;
	}
	return NULL;
}

void aig_Close(aig_t *aig)
{
	free(aig);
}

/*========================================*/

/* returns position or -1 */
int tcg_AddPlainAig(tcg_t *tcg)
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


int tcg_AddAig(tcg_t *tcg, int tig_src, int dir_src, int pos_src, int tig_dest, int dir_dest, int pos_dest)
{
	int idx;
	aig_t *aig;
	idx = tcg_AddPlainAig(tcg);
	if ( idx >= 0 )
	{
		aig = tcg_GetAig(tcg, idx);
		
		aig->tig_src = tig_src;
		aig->dir_src = dir_src;
		aig->pos_src = pos_src;
		
		aig->tig_dest = tig_dest;
		aig->dir_dest = dir_dest;
		aig->pos_dest = pos_dest;
		
		tcg_CalculateAigPath(tcg, idx);
		tcg_ShowAigPoints(tcg, idx);

		return idx;
	}
	return -1;
}


void tcg_DeselectAig(tcg_t *tcg, int aig_idx)
{
	aig_t *aig;
	int i, cnt;
	aig = tcg_GetAig(tcg, aig_idx);
	
	cnt = aig->dfv_cnt;
	for( i = 0; i < cnt; i++ )
	{
		aig->dfv_list[i].is_selected = 0;
	}
}

void tcg_DeleteAig(tcg_t *tcg, int aig_idx)
{
	aig_t *aig;
	int i, cnt;
	aig = tcg_GetAig(tcg, aig_idx);
	aig_Close(aig);	
	ps_Del(tcg->aig_list, aig_idx);
}


