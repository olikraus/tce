/*

	tcg.h
	
	Tool Chain Editor, graph procedures
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

/*========================================*/
/* forward typedefs */

typedef struct tcgv_struct tcgv_t;
typedef struct tig_struct tig_t;
typedef struct tcg_struct tcg_t;
typedef struct rect_struct rect_t;

/*========================================*/
/* struct declaratons */

struct rect_struct
{
	long x0, y0;	/* upper left */
	long x1, y1;	/* lower right */
};

/* a view on a tool chain graph */
struct tcgv_struct
{
	tcg_t *tcg;	/* referes to the parent */
	long x;		/* upper left position of the viewport (graph coordinates) */
	long y;	
	double zoom;	/* zoom factor */
};

/* tool in graph */
struct tig_struct
{
	char *name;
	rect_t area;		/* the current rectangle (position and size) which covers this element */
	rect_t move_start_area;	/* backup of the current position and size when move has started */
	
	unsigned int is_selected; /* tig is selected */
};

/* the tool chain graph */
struct tcg_struct
{
	pl_t *tig_list;
	/* dimensions of the graph */
	rect_t graph_dimension;

	
	tcgv_t *tcgv;

	/* this is the catch area, all elements which do have a none-zero intersection are "catched" */	
	rect_t catch_area;
	
	int state;
	long start_x;	/* start position of the catch area or movement */
	long start_y;
	
};

#define TCG_STATE_IDLE 0
#define TCG_STATE_ADD_CATCH_AREA 1
#define TCG_STATE_REPLACE_CATCH_AREA 2
#define TCG_STATE_DO_CATCH_AREA_SELECTION 3
#define TCG_STATE_SINGLE_MOVE 4
#define TCG_STATE_SELECTON_MOVE 5
#define TCG_STATE_DO_MOVEMENT 6

#define TCG_EVENT_BUTTON_DOWN 0
#define TCG_EVENT_SHIFT_BUTTON_DOWN 1
#define TCG_EVENT_MOUSE_MOVE 2
#define TCG_EVENT_BUTTON_UP 3


#define tcg_IsMovement(tcg) ((tcg)->state == TCG_STATE_DO_MOVEMENT)
#define tcg_IsCatchAreaVisible(tcg) ((tcg)->state == TCG_STATE_DO_CATCH_AREA_SELECTION)


/*========================================*/
/* tcg procedures */

#define tcg_GetTIgCnt(tcg) pl_Cnt((tcg)->tig_list)
#define tcg_WhileTig(tcg, idx) pl_While((tcg)->tig_list, (idx))
#define tcg_GetTig(tcg, idx) ((tig_t *)pl_Get((tcg)->tig_list, (idx)))
#define tcg_SetTig(tcg, idx, tig) (pl_Set((tcg)->tig_list, (idx), (void *)(tig)))

void tgc_CalculateDimension(tcg_t *tcg);
long tgc_GetGraphXFromView(tcg_t *tcg, double x);
long tgc_GetGraphYFromView(tcg_t *tcg, double y);
double tgc_GetViewXFromGraph(tcg_t *tcg, long x);
double tgc_GetViewYFromGraph(tcg_t *tcg, long y);

tcg_t *tcg_Open(void);
void tcg_Close(tcg_t *tcg);
void tcg_DeleteTig(tcg_t *tcg, int idx);
int tcg_AddTig(tcg_t *tcg, const char *name, long x, long y);
int tcg_CatchElement(tcg_t *tcg, double x, double y);
int tcg_IsCatched(tcg_t *tcg, int idx);
int tcg_IsSelected(tcg_t *tcg, int idx);
int tcg_SendEventWithGraphPosition(tcg_t *tcg, int event, long x, long y);
int tcg_SendEventWithViewPosition(tcg_t *tcg, int event, double x, double y);


