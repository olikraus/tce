/*

	ps.h

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

#ifndef _ps_H
#define _ps_H

#include <stddef.h>

struct _ps_struct
{
  void **list_ptr;
  int list_max;
  int list_cnt;

  int search_pos_start;
};
typedef struct _ps_struct ps_t;

#define ps_Get(ps, pos) ((ps)->list_ptr[(pos)])
#define ps_Cnt(ps) ((ps)->list_cnt)

ps_t *ps_Open();
void ps_Clear(ps_t *ps);
void ps_Close(ps_t *ps);
int ps_IsValid(ps_t *ps, int pos);
/* returns access handle or -1 */
int ps_Add(ps_t *ps, void *ptr);
int ps_Set(ps_t *ps, int pos, void *ptr);
void ps_Del(ps_t *ps, int pos);
int ps_Next(ps_t *ps, int pos);
int ps_First(ps_t *ps);
int ps_WhileLoop(ps_t *ps, int *pos);

size_t ps_GetMemUsage(ps_t *ps);

#endif
