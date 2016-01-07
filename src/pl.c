/*
	pl.c

	Tool Chain Editor
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
#include "pl.h"

#include <stdlib.h>
#include <assert.h>

#define PL_EXPAND 32  
  
int pl_init(pl_t *pl)
{
  pl->cnt = 0;
  pl->max = 0;
  pl->list = (void **)malloc(PL_EXPAND*sizeof(void *));
  if ( pl->list == NULL )
    return 0;
  pl->max = PL_EXPAND;
  return 1;
}

int pl_expand(pl_t *pl)
{
  void *ptr;
  ptr = realloc(pl->list, (pl->max+PL_EXPAND)*sizeof(void *));
  if ( ptr == NULL )
    return 0;
  pl->list = (void **)ptr;
  pl->max += PL_EXPAND;
  return 1;
}

void pl_destroy(pl_t *pl)
{
  pl_Clear(pl);
  free(pl->list);
  pl->max = 0;
}

/*----------------------------------------------------------------------------*/

pl_t *pl_Open(void)
{
  pl_t *pl;
  pl = (pl_t *)malloc(sizeof(pl_t));
  if ( pl != NULL )
  {
    if ( pl_init(pl) != 0 )
    {
      return pl;
    }
    free(pl);
  }
  return NULL;
}

void pl_Close(pl_t *pl)
{
  pl_destroy(pl);
  free(pl);
}

/* returns position or -1 */
int pl_Add(pl_t *pl, void *ptr)
{
  while( pl->cnt >= pl->max )
    if ( pl_expand(pl) == 0 )
      return -1;
  
  pl->list[pl->cnt] = ptr;
  pl->cnt++;
  return pl->cnt-1;
}

void pl_DelByPos(pl_t *pl, int pos)
{
  while( pos+1 < pl->cnt )
  {
    pl->list[pos] = pl->list[pos+1];
    pos++;
  }
  if ( pl->cnt > 0 )
    pl->cnt--;
}

/* 0: error */
int pl_InsByPos(pl_t *pl, void *ptr, int pos)
{
  int i;
  if ( pl_Add(pl, ptr) < 0 )
    return 0;
  assert(pl->cnt > 0 );
  i = pl->cnt-1;
  while( i > pos + 4 )
  {
    pl->list[i-0] = pl->list[i-1];
    pl->list[i-1] = pl->list[i-2];
    pl->list[i-2] = pl->list[i-3];
    pl->list[i-3] = pl->list[i-4];
    i-=4;
  }
  while( i > pos )
  {
    pl->list[i] = pl->list[i-1];
    i--;
  }
  pl->list[i] = ptr;
  return 1;
}

int pl_While(pl_t *pl, int *pos)
{
  if ( pl->cnt == 0 )
  {
    *pos = -1;
    return 0;
  }
  else if ( *pos < 0 )
  {
    *pos = 0;
    return 1;
  }
  else if ( *pos+1 >= pl->cnt )
  {
    *pos = -1;
    return 0;
  }
  (*pos)++;
  return 1;
}



/*----------------------------------------------------------------------------*/

void pl_Sort(pl_t *pl, int (*compar)(const void *, const void *))
{
  qsort(pl->list, pl->cnt, sizeof(void *), compar);
}

/*----------------------------------------------------------------------------*/

size_t pl_GetMemUsage(pl_t *pl)
{
  size_t m;
  m = sizeof(pl_struct);
  m += sizeof(void *)*pl->max;
  return m;
}
