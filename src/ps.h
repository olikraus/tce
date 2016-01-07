/*

  ps.h

  (c) 2016 by Oliver Kraus, olikraus@yahoo.com

  A set with pointer values (pointer set)

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
