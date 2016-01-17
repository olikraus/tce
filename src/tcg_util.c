/*

	tcg_util.c
	
	Tool Chain Editor, utility procedures
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
#include <string.h>
#include <limits.h>
#include "tcg.h"


int tce_strset(char **s, const char *t)
{
	if ( s == NULL )
		return 0;
	if ( *s != NULL )
		free(*s);
	*s = NULL;
	
	if ( t != NULL )
	{
		*s = strdup(t);
		if ( *s == NULL )
			return 0;
	}
	return 1;
}

/* 
	check whether the line a0, a1 intersecs with v0, v1

	precondition 
		1) a0 <= a1
		2) b0 <= b1
*/

int is_intersection(long a0, long a1, long b0, long b1) 
{
	if ( a1 < b0 )
		return 0;
	if ( a0 > b1 )
		return 0;
	return 1;
}

int is_rectangle_intersection(const rect_t *a, const rect_t *b)
{
	if ( is_intersection(a->x0, a->x1, b->x0, b->x1) == 0 )
		return 0;
	if ( is_intersection(a->y0, a->y1, b->y0, b->y1) == 0 )
		return 0;
	return 1;
}

void clear_rectangle(rect_t *r)
{
	r->x0 = 0;
	r->x1 = 0;
	r->y0 = 0;
	r->y1 = 0;
}

void invalid_rectangle(rect_t *r)
{
	r->x0 = 1;
	r->x1 = 0;
	r->y0 = 1;
	r->y1 = 0;
}

void expand_rectangle(rect_t *to_be_expanded, const rect_t *r)
{
	if ( to_be_expanded->x0 > r->x0 )
		to_be_expanded->x0 = r->x0;
	if ( to_be_expanded->x1 < r->x1 )
		to_be_expanded->x1 = r->x1;
	if ( to_be_expanded->y0 > r->y0 )
		to_be_expanded->y0 = r->y0;
	if ( to_be_expanded->y1 < r->y1 )
		to_be_expanded->y1 = r->y1;	
}


