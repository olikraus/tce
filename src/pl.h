/*
	pl.h

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

/*

  pl.h

   pointer list

  
*/

#ifndef _PL_H
#define _PL_H

#include <stddef.h>
#include <stdio.h>

struct _pl_struct
{
  int cnt;
  int max;
  void **list;
};
typedef struct _pl_struct pl_struct;
typedef struct _pl_struct *pl_type;
typedef struct _pl_struct pl_t;

int pl_init(pl_t *pl);
int pl_expand(pl_t *pl);
void pl_destroy(pl_t *pl);

#define pl_Cnt(pl)         ((pl)->cnt)
#define pl_Get(pl,pos)     ((pl)->list[pos])
#define pl_Set(pl,pos,val) ((pl)->list[pos] = (val))
#define pl_Clear(pl)          ((pl)->cnt = 0)


pl_t *pl_Open();
void pl_Close(pl_t *pl);
/* returns position or -1 */
int pl_Add(pl_t *pl, void *ptr);
void pl_DelByPos(pl_t *pl, int pos);
int pl_InsByPos(pl_t *pl, void *ptr, int pos);
int pl_While(pl_t *pl, int *pos);

int pl_Write(pl_t *pl, FILE *fp, int (*write_el)(FILE *fp, void *el, void *ud), void *ud);
int pl_Read(pl_t *pl, FILE *fp, void *(*read_el)(FILE *fp, void *ud), void *ud);

void pl_Sort(pl_t *pl, int (*compar)(const void *, const void *));

size_t pl_GetMemUsage(pl_t *pl);

#endif

