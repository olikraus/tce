/*

	tcg_seg.c
	
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

