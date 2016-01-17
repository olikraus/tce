/*

	tcg_connect.c
	
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
	
	
	L: length (width or height) of the box
	
	number of connects:
		T = ((L-2*GRID_SIZE) / GRID_SIZE)/3
		if ( T == 0 )
			return 1;
		return T * 3;
	
	
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include "tcg.h"


int tcg_GetConnectCnt(tcg_t *tcg, int idx, int dir)
{
	long l;
	tig_t *eig;
	if ( idx < 0 )
		return 0;	/* error */
	eig = tcg_GetTig(tcg, idx);
	if ( eig == NULL )
		return 0;	/* error */
	
	if ( (dir & 1) == 0 )
	{
		l = tig_GetHeight(eig);
	}
	else
	{
		l = tig_GetWidth(eig);
	}
	
	l = (( l -2 * TCG_CONNECT_GRID_SIZE ) / TCG_CONNECT_GRID_SIZE ) / 3;
	if ( l == 0 )
		return 1;
	return l * 3;
}

long tcg_GetConnectDeltaPos(tcg_t *tcg, int idx, int dir, int pos)
{
	long l, m;
	tig_t *eig;
	if ( idx < 0 )
		return 0;	/* error */
	eig = tcg_GetTig(tcg, idx);
	if ( eig == NULL )
		return 0;	/* error */
	
	if ( (dir & 1) == 0 )
	{
		l = tig_GetHeight(eig);
	}
	else
	{
		l = tig_GetWidth(eig);
	}
	m = l / 2;
	if ( pos == 0 )
		return m;
	
	if ( (pos & 1) == 1 )
	{
		pos /= 2;
		pos++;
		return m + pos * TCG_CONNECT_GRID_SIZE;
	}
	else
	{
		pos /= 2;
		assert(m - pos * TCG_CONNECT_GRID_SIZE >= 0 );
		return m - pos * TCG_CONNECT_GRID_SIZE;
	}
}

long tcg_GetConnectPosX(tcg_t *tcg, int idx, int dir, int pos)
{
	tig_t *eig;
	if ( idx < 0 )
		return 0;	/* error */
	eig = tcg_GetTig(tcg, idx);
	if ( eig == NULL )
		return 0;	/* error */

	if ( dir == 0 )
	{
		return eig->area.x1;
	}
	else if ( dir == 1 )
	{
		return eig->area.x0 + tcg_GetConnectDeltaPos(tcg, idx, dir, pos);		
	}
	else if ( dir == 2 )
	{
		return eig->area.x0;
	}
	else 
	{
		return eig->area.x0 + tcg_GetConnectDeltaPos(tcg, idx, dir, pos);
	}
	
}

long tcg_GetConnectPosY(tcg_t *tcg, int idx, int dir, int pos)
{
	tig_t *eig;
	if ( idx < 0 )
		return 0;	/* error */
	eig = tcg_GetTig(tcg, idx);
	if ( eig == NULL )
		return 0;	/* error */

	if ( dir == 0 )
	{
		return eig->area.y0 + tcg_GetConnectDeltaPos(tcg, idx, dir, pos);
	}
	else if ( dir == 1 )
	{
		return eig->area.y1;
	}
	else if ( dir == 2 )
	{
		return eig->area.y0 + tcg_GetConnectDeltaPos(tcg, idx, dir, pos);
	}
	else 
	{
		return eig->area.y0;
	}
	
}

/*
	checks, whether the connector at "dir" and "pos" is catched
	tcg_IsTigCatched(tcg, idx) should also return 1 (however, this is not checked here)

	additionally, this check should not be executed if tcg_IsCatchAreaVisible(tcg) returns true
*/
int tcg_IsConnectCatched(tcg_t *tcg, int idx, int dir, int pos)
{
	rect_t c;
	
	c.x0 = tcg_GetConnectPosX(tcg, idx, dir, pos);
	c.y0 = tcg_GetConnectPosY(tcg, idx, dir, pos);
	c.x1 = c.x0;
	c.y1 = c.y0;
	c.x0-=TCG_CONNECT_HALF_WIDTH;
	c.y0-=TCG_CONNECT_HALF_WIDTH;
	c.x1+=TCG_CONNECT_HALF_WIDTH;
	c.y1+=TCG_CONNECT_HALF_WIDTH;
	
	return is_rectangle_intersection(&c, &(tcg->catch_area));
}

/*
	find a catched connector
	return 0 if no connector was found
	This function will always return 0 if tcg_IsCatchAreaVisible(tcg) is active (multi catch)

*/
int tcg_GetCatchedConnect(tcg_t *tcg, int idx, int *dir_p, int *pos_p)
{
	int dir;
	int pos;
	int cnt;
	
	if ( tcg_IsCatchAreaVisible(tcg)  != 0 )
		return 0;		/* do not catch a connector in multi catch mode */
	
	if ( tcg_IsTigCatched(tcg, idx) == 0 )
		return 0;		/* master element is not chatched */
	
	for( dir = 0; dir < 4; dir++ )
	{
		cnt = tcg_GetConnectCnt(tcg, idx, dir);
		for ( pos = 0; pos < cnt; pos++ )
		{
			if ( tcg_IsConnectCatched(tcg, idx, dir, pos) != 0 )
			{
				*dir_p = dir;
				*pos_p = pos;
				return 1;
			}
		}
	}	
	return 0;
}

int tcg_GetCatchedConnectRect(tcg_t *tcg, int tig_idx, rect_t *r)
{
	int dir, pos;
	long x, y;
	if ( tcg_GetCatchedConnect(tcg, tig_idx, &dir, &pos) != 0 )
	{
		x = tcg_GetConnectPosX(tcg, tig_idx, dir, pos);
		y = tcg_GetConnectPosY(tcg, tig_idx, dir, pos);
		r->x0 = x-TCG_CONNECT_HALF_WIDTH;
		r->x1 = x+TCG_CONNECT_HALF_WIDTH;
		r->y0 = y-TCG_CONNECT_HALF_WIDTH;
		r->y1 = y+TCG_CONNECT_HALF_WIDTH;
		return 1;
	}
	return 0;
}




