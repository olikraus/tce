/*

	tcg_seg.c
	
	OBSOLETE
	
	Tool Chain Editor, path segment related function
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

seg_t *seg_Open(void)
{
  seg_t *s;
  s = (seg_t *)malloc(sizeof(seg_t));
  if ( s != NULL )
  {
    s->aig_idx = -1;
    s->seg_seq_no = -1;
    return s;
  }
  return NULL;
}

void seg_Close(seg_t *seg)
{
  free(seg);
}

/*========================================*/


/* returns position or -1 */
int tcg_AddPlainSeg(tcg_t *tcg)
{
	seg_t *seg;
	int idx;
	seg = seg_Open();
	if ( seg != NULL )
	{
		idx = ps_Add(tcg->seg_list, seg);
		if ( idx >= 0 )
		{
			return idx;
		}
		seg_Close(seg);
	}
	return -1;
}

/* returns position or -1 */
int tcg_AddSeg(tcg_t *tcg, int aig_idx, int seg_seq_no)
{
	int idx;
	seg_t *seg;
	idx = tcg_AddPlainSeg(tcg);
	if ( idx >= 0 )
	{
		seg = tcg_GetSeg(tcg, idx);
		seg->aig_idx = aig_idx;
		seg->seg_seq_no = seg_seq_no;
	}
	return idx;
	
}


/*
  delete all segments which belong to the speciofied artefact index
*/
void tcg_DeleteSegPathByAig(tcg_t *tcg, int aig_idx)
{
	int i;
	seg_t *seg;
	
	i = -1;
	while( tcg_WhileSeg(tcg, &i) )
	{
		seg = tcg_GetSeg(tcg, i);
		if ( seg->aig_idx == aig_idx )
		{
		  seg_Close(seg);
		  ps_Del(tcg->seg_list, i);
		}
	}
  
}

void tcg_CreateSegFromAig(tcg_t *tcg, int aig_idx)
{
	int cnt;
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	
	cnt = aig->dfv_cnt+2;		/* number of segments is two more than the number of independent variables */
	
}



