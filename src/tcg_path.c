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

int tcg_GetAigPointCnt(tcg_t *tcg, int aig_idx)
{
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	
	return aig->dfv_cnt+3;
}

long tcg_GetAigPointX(tcg_t *tcg, int aig_idx, int pnt_idx)
{
	long x;
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	
	if ( pnt_idx == 0 )
	{
		x = tcg_GetConnectPosX(tcg, aig->tig_src, aig->dir_src, aig->pos_src);
	}
	else if ( pnt_idx == aig->dfv_cnt+2 )
	{
		x = tcg_GetConnectPosX(tcg, aig->tig_dest, aig->dir_dest, aig->pos_dest);
	}
	else if ( pnt_idx == aig->dfv_cnt+1 )
	{
		if ( (aig->dir_dest & 1) != 0 )
			x = tcg_GetConnectPosX(tcg, aig->tig_dest, aig->dir_dest, aig->pos_dest);
		else
			x = tcg_GetAigPointX(tcg, aig_idx, pnt_idx - 1);			
	}
	else
	{
		if ( aig->dfv_list[pnt_idx-1].is_vertical != 0 )
			x = aig->dfv_list[pnt_idx-1].v;
		else
			x = tcg_GetAigPointX(tcg, aig_idx, pnt_idx - 1);
	}
	return x;

}

long tcg_GetAigPointY(tcg_t *tcg, int aig_idx, int pnt_idx)
{
	long y;
	aig_t *aig;
	aig = tcg_GetAig(tcg, aig_idx);
	
	if ( pnt_idx == 0 )
	{
		y = tcg_GetConnectPosY(tcg, aig->tig_src, aig->dir_src, aig->pos_src);
	}
	else if ( pnt_idx == aig->dfv_cnt+2 )
	{
		y = tcg_GetConnectPosY(tcg, aig->tig_dest, aig->dir_dest, aig->pos_dest);
	}
	else if ( pnt_idx == aig->dfv_cnt+1 )
	{
		if ( (aig->dir_dest & 1) == 0 )
			y = tcg_GetConnectPosY(tcg, aig->tig_dest, aig->dir_dest, aig->pos_dest);
		else
			y = tcg_GetAigPointY(tcg, aig_idx, pnt_idx - 1);
	}
	else
	{
		if ( aig->dfv_list[pnt_idx-1].is_vertical == 0 )
			y = aig->dfv_list[pnt_idx-1].v;
		else
			y = tcg_GetAigPointY(tcg, aig_idx, pnt_idx - 1);
	}
	return y;

}


#ifdef OBSOLETE
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


#endif
void tcg_ShowAigPoints(tcg_t *tcg, int aig_idx)
{
	int i, cnt;
	cnt = tcg_GetAigPointCnt(tcg, aig_idx);
	for( i = 0; i < cnt; i++ )
	{
		printf("%d/%d  ", i, cnt-1 );
		printf("x: %ld  ", tcg_GetAigPointX(tcg, aig_idx, i) );
		printf("y: %ld  ", tcg_GetAigPointY(tcg, aig_idx, i) );
		printf("\n");
	}
	
}

void tcg_CalculateAigParallelPath(tcg_t *tcg, int idx)
{
	int dir;
	aig_t *aig;
	//tig_t *eig_src;
	//tig_t *eig_dest;
	
	//eig_src = tcg_GetTig(tcg, aig->tig_src);
	//eig_dest = tcg_GetTig(tcg, aig->tig_dest);
	
	aig = tcg_GetAig(tcg, idx);
	/* aig->dir_src == aig->dir_dest */
	dir = aig->dir_src;
	aig->dfv_cnt = 1;		/* only one degree of freedom */
	//aig->point_val_cnt = 2;	/* two points required */
	
	if ( dir == 0 || dir == 2 )
	{
		/* y is fixed, x will be variable */
		if ( dir == 0 )
		{
			printf("tcg_CalculateAigParallelPath: dir == 0\n");
			/* dir == 0 */
			aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->tig_src,  dir);
			if ( aig->dfv_list[0].min < tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir) )
				aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir);
			
			aig->dfv_list[0].max = LONG_MAX;
			aig->dfv_list[0].v = aig->dfv_list[0].min + 5;
			aig->dfv_list[0].is_vertical = (dir+1)&1;

			
			/* point 0, closer to src */
			//aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */
			//aig->dfv_ref_list[1] = AIG_DFV_MAX;		/* y: fixed with source */

			/* point 1, connected to dest */
			//aig->dfv_ref_list[2] = 0;				/* x: use dfv_list[0] */
			//aig->dfv_ref_list[3] = AIG_DFV_MAX+1;	/* y: fixed with dest */
		}
		else
		{
			/* dir == 2 */
			aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->tig_src,  dir);
			if ( aig->dfv_list[0].max > tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir) )
				aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir);

			aig->dfv_list[0].max = LONG_MIN;
			aig->dfv_list[0].v = aig->dfv_list[0].max - 5;
			aig->dfv_list[0].is_vertical = (dir+1)&1;

			/* point 0, closer to src */
			//aig->dfv_ref_list[0] = 0;				/* x: use dfv_list[0] */
			//aig->dfv_ref_list[1] = AIG_DFV_MAX;		/* y: fixed with source */

			/* point 1, connected to dest */
			//aig->dfv_ref_list[2] = 0;				/* x: use dfv_list[0] */
			//aig->dfv_ref_list[3] = AIG_DFV_MAX+1;	/* y: fixed with dest */
			
		}
	}
	else
	{
		if ( dir == 1 )
		{
			/* dir == 1 */
			aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->tig_src,  dir);
			if ( aig->dfv_list[0].min < tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir) )
				aig->dfv_list[0].min = tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir);
			
			aig->dfv_list[0].max = LONG_MAX;
			aig->dfv_list[0].v = aig->dfv_list[0].min + 5;
			aig->dfv_list[0].is_vertical = (dir+1)&1;

			/* point 0, closer to src */
			//aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* x: fixed with source */
			//aig->dfv_ref_list[1] = 0;				/* y: use dfv_list[0] */

			/* point 1, connected to dest */
			//aig->dfv_ref_list[2] = AIG_DFV_MAX+1;	/* x: fixed with dest */
			//aig->dfv_ref_list[3] = 0;				/* y: use dfv_list[0] */
			
		}
		else
		{
			/* dir == 3 */
			aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->tig_src,  dir);
			if ( aig->dfv_list[0].max > tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir) )
				aig->dfv_list[0].max = tcg_GetTigBorderValueByDir(tcg,  aig->tig_dest,  dir);

			aig->dfv_list[0].max = LONG_MIN;
			aig->dfv_list[0].v = aig->dfv_list[0].max - 5;
			aig->dfv_list[0].is_vertical = (dir+1)&1;
			
			/* point 0, closer to src */
			//aig->dfv_ref_list[0] = AIG_DFV_MAX;		/* x: fixed with source */
			//aig->dfv_ref_list[1] = 0;				/* y: use dfv_list[0] */

			/* point 1, connected to dest */
			//aig->dfv_ref_list[2] = AIG_DFV_MAX+1;	/* x: fixed with dest */
			//aig->dfv_ref_list[3] = 0;				/* y: use dfv_list[0] */
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

