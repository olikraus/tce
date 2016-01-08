/*

	ps.c

	A set with pointer values (pointer set)

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


#include <stdlib.h>
#include <assert.h>
#include "ps.h"

#define PS_EXPAND 16

ps_t *ps_Open()
{
  ps_t *ps;
  int i;
  ps = (ps_t *)malloc(sizeof(ps_t));
  if ( ps != NULL )
  {
    ps->list_ptr = (void **)malloc(PS_EXPAND*sizeof(void *));
    if ( ps->list_ptr != NULL )
    {
      for( i = 0; i < PS_EXPAND; i++ )
        ps->list_ptr[i] = NULL;
      ps->list_max = PS_EXPAND;
      ps->list_cnt = 0;
      ps->search_pos_start = 0;
      return ps;
    }
    free(ps);
  }
  return NULL;
}

void ps_Clear(ps_t *ps)
{
  int i;
  for( i = 0; i < ps->list_max; i++  )
    ps->list_ptr[i] = NULL;
  ps->list_cnt = 0;
}

void ps_Close(ps_t *ps)
{
  ps_Clear(ps);
  assert ( ps->list_ptr != NULL );
  free(ps->list_ptr);
  free(ps);
}

int ps_IsValid(ps_t *ps, int pos)
{
  if ( pos < 0 )
    return 0;
  if ( pos >= ps->list_max )
    return 0;
  if ( ps->list_ptr[pos] == NULL )
    return 0;
  return 1;
}

static int _ps_expand(ps_t *ps)
{
  void *ptr;
  int pos;
  assert(ps != NULL);
  assert(ps->list_ptr != NULL);
  assert((ps->list_max % PS_EXPAND) == 0);
  
  ptr = (void *)realloc(ps->list_ptr, sizeof(void *)*(ps->list_max + PS_EXPAND));
  if ( ptr == NULL )
    return 0;
  
  ps->list_ptr = ptr;
  pos = ps->list_max;
  ps->search_pos_start = ps->list_max;
  ps->list_max += PS_EXPAND;
  while( pos < ps->list_max )
  {
    ps->list_ptr[pos] = NULL;
    pos++;
  }
  return 1;
}


static int ps_find_empty(ps_t *ps)
{
  int i;
  for( i = ps->search_pos_start; i < ps->list_max; i++ )
  {
    if ( ps->list_ptr[i] == NULL )
    {
      ps->search_pos_start = i + 1;
      if ( ps->search_pos_start >= ps->list_max )
        ps->search_pos_start = 0;
      return i;
    }
  }
  assert(ps->search_pos_start < ps->list_max);
  for( i = 0; i < ps->search_pos_start; i++ )
  {
    if ( ps->list_ptr[i] == NULL )
    {
      ps->search_pos_start = i + 1;
      if ( ps->search_pos_start >= ps->list_max )
        ps->search_pos_start = 0;
      return i;
    }
  }
  return -1;
}

/* returns access handle or -1 */
int ps_Add(ps_t *ps, void *ptr)
{
  int pos;

  assert(ps != NULL);
  assert(ps->list_ptr != NULL);
  assert(ptr != NULL);
  assert(ps->list_max >= ps->list_cnt);
  
  pos = ps_find_empty(ps);
  if ( pos < 0 )
  {
    if ( _ps_expand(ps) == 0 )
      return -1;
    pos = ps_find_empty(ps);
    if ( pos < 0 )
      return -1;
  }
  
  ps->list_ptr[pos] = ptr;
  ps->list_cnt++;
  return pos;
}

int ps_Set(ps_t *ps, int pos, void *ptr)
{
  while(ps->list_max <= pos)
    if ( _ps_expand(ps) == 0 )
      return 0;
  assert(pos >= 0);
  assert(pos < ps->list_max);
  if ( ps->list_ptr[pos] == NULL && ptr != NULL )
    ps->list_cnt++;
  if ( ps->list_ptr[pos] != NULL && ptr == NULL )
    ps->list_cnt--;
  ps->list_ptr[pos] = ptr;
  return 1;
}

/* delete a pointer by its access handle */
void ps_Del(ps_t *ps, int pos)
{
  assert(pos >= 0);
  assert(pos < ps->list_max);
  assert(ps->list_ptr[pos] != NULL);
  assert(ps->list_cnt > 0 );
  ps->list_ptr[pos] = NULL;
  ps->list_cnt--;
}

int ps_Next(ps_t *ps, int pos)
{
  for(;;)
  {
    pos++;
    if ( pos >= ps->list_max )
      return -1;
    if ( ps->list_ptr[pos] != NULL )
      return pos;
  }
}

int ps_First(ps_t *ps)
{
  return ps_Next(ps, -1);
}

/* *pos must be initilized with -1 */
int _ps_WhileLoop(ps_t *ps, int *pos)
{
  *pos = ps_Next(ps, *pos);
  if ( *pos < 0 )
    return 0;
  return 1;
}

/* *pos must be initilized with -1 */
int ps_WhileLoop(ps_t *ps, int *pos_ptr)
{
  int pos = *pos_ptr;
  
  for(;;)
  {
    pos++;
    if ( pos >= ps->list_max )
    {
      *pos_ptr = -1;
      return 0;
    }
    if ( ps->list_ptr[pos] != NULL )
    {
      *pos_ptr = pos;
      return 1;
    }
  }
}

size_t ps_GetMemUsage(ps_t *ps)
{
  size_t m = sizeof(ps_t);
  m += sizeof(void *)*ps->list_max;
  return m;
}

