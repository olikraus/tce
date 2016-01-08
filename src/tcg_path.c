/*

	tcg_path.c
	
	Tool Chain Editor, automatic path calculation
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
#include <math.h>
#include "tcg.h"


/*
	depending on dir, return a suitable value from the area struct of the tig
*/
long tcg_GetTigBorderValueByDir(tcg_t *tcg, int idx, int dir)
{
	tig_t *eig;
	eig = tcg_GetTig(tcg, idx);
	switch( dir )
	{
		case 0: return eig->area.x1; 
		case 1: return eig->area.y1; 
		case 2: return eig->area.x0; 
		case 3: return eig->area.y0; 
	}
	return 0;
}

long tcg_GetAigPointByDFIdx(tcg_t *tcg, int aig_idx, int dfv_idx)
{
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	
	if ( dfv_idx == AIG_DFV_MAX ) 
	{
		return tcg_GetTigBorderValueByDir(tcg,  aig->eig_src,  aig->dir_src);
	}
	else if ( dfv_idx == AIG_DFV_MAX+1 ) 
	{
		return tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  aig->dir_dest);
	}
	else
	{
		return aig->dfv_list[dfv_idx].v;
	}
}


long tcg_GetAigPointX(tcg_t *tcg, int aig_idx, int pnt_idx)
{
	int dfv_idx;
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	dfv_idx = aig->dfv_ref_list[pnt_idx*2];
	return tcg_GetAigPointByDFIdx(tcg, aig_idx, dfv_idx);
}

long tcg_GetAigPointY(tcg_t *tcg, int aig_idx, int pnt_idx)
{
	int dfv_idx;
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	dfv_idx = aig->dfv_ref_list[pnt_idx*2+1];
	return tcg_GetAigPointByDFIdx(tcg, aig_idx, dfv_idx);
}


void tcg_ShowAigPoints(tcg_t *tcg, int aig_idx)
{
	aig_t *aig;
	int i;
	aig = tcg_GetAig(tcg, aig_idx);
	for( i = 0; i < aig->point_val_cnt; i++ )
	{
		printf("x: dfv_ref_list[%d]=%d val=%ld  ", i*2, aig->dfv_ref_list[i*2], tcg_GetAigPointX(tcg, aig_idx, i) );
		printf("y: dfv_ref_list[%d]=%d val=%ld  ", i*2+1, aig->dfv_ref_list[i*2+1], tcg_GetAigPointY(tcg, aig_idx, i) );
		printf("\n");
	}
	
}

void tcg_CalculateAigParallelPath(tcg_t *tcg, int idx)
{
	int dir;
	aig_t *aig;
	tig_t *eig_src;
	tig_t *eig_dest;
	aig = tcg_GetAig(tcg, idx);
	
	eig_src = tcg_GetTig(tcg, aig->eig_src);
	eig_dest = tcg_GetTig(tcg, aig->eig_dest);
	
	/* aig->dir_src == aig->dir_dest */
	dir = aig->dir_src;
	aig->dfv_cnt = 1;		/* only one degree of freedom */
	aig->point_val_cnt = 2;	/* two points required */
	
	if ( dir == 0 || dir == 2 )
	{
		/* y is fixed, x will be variable */
		if ( dir == 0 )
		{
			/* dir == 0 */
			aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->eig_src,  dir);
			if ( aig->dfv_list[0].min < tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir) )
				aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir);
			
			aig->dfv_list[0].max = LONG_MAX;
			aig->dfv_list[0].v = aig->dfv_list[0].min + 5;

			/* point 0, closer to src */
			aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */
			aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* y: fixed with source */

			/* point 1, connected to dest */
			aig->dfv_ref_list[1] = 0;				/* x: use dfv_list[0] */
			aig->dfv_ref_list[1] = AIG_DFV_MAX+1;	/* y: fixed with dest */
		}
		else
		{
			/* dir == 2 */
			aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->eig_src,  dir);
			if ( aig->dfv_list[0].max > tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir) )
				aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir);

			aig->dfv_list[0].max = LONG_MIN;
			aig->dfv_list[0].v = aig->dfv_list[0].max - 5;

			/* point 0, closer to src */
			aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */
			aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* y: fixed with source */

			/* point 1, connected to dest */
			aig->dfv_ref_list[1] = 0;				/* x: use dfv_list[0] */
			aig->dfv_ref_list[1] = AIG_DFV_MAX+1;	/* y: fixed with dest */
			
		}
	}
	else
	{
		if ( dir == 1 )
		{
			/* dir == 1 */
			aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->eig_src,  dir);
			if ( aig->dfv_list[0].min < tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir) )
				aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir);
			
			aig->dfv_list[0].max = LONG_MAX;
			aig->dfv_list[0].v = aig->dfv_list[0].min + 5;

			/* point 0, closer to src */
			aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* y: fixed with source */
			aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */

			/* point 1, connected to dest */
			aig->dfv_ref_list[1] = AIG_DFV_MAX+1;	/* y: fixed with dest */
			aig->dfv_ref_list[1] = 0;				/* x: use dfv_list[0] */
			
		}
		else
		{
			/* dir == 3 */
			aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->eig_src,  dir);
			if ( aig->dfv_list[0].max > tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir) )
				aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->eig_dest,  dir);

			aig->dfv_list[0].max = LONG_MIN;
			aig->dfv_list[0].v = aig->dfv_list[0].max - 5;
			
			/* point 0, closer to src */
			aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* y: fixed with source */
			aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */

			/* point 1, connected to dest */
			aig->dfv_ref_list[1] = AIG_DFV_MAX+1;	/* y: fixed with dest */
			aig->dfv_ref_list[1] = 0;				/* x: use dfv_list[0] */
		}
	}
}

void tcg_CalculateAigAntiParallelPath(tcg_t *tcg, int idx)
{
}

void tcg_CalculateAigEdgePath(tcg_t *tcg, int idx)
{
}


void tcg_CalculateAigPath(tcg_t *tcg, int idx)
{
	aig_t *aig;
	aig = tcg_GetAig(tcg, idx);
	
	if ( aig->dir_src == aig->dir_dest )
	{
		tcg_CalculateAigParallelPath(tcg, idx);
	}
	else if ( aig->dir_src + aig->dir_dest == 4 )
	{
		tcg_CalculateAigAntiParallelPath(tcg, idx);
	}
	else
	{
		tcg_CalculateAigEdgePath(tcg, idx);
	}
}

