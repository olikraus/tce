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


#include "ps.h"

/*========================================*/
/* forward typedefs */

typedef struct tcgv_struct tcgv_t;
typedef struct tig_struct tig_t;
typedef struct aig_struct aig_t;
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

/* degree of freedom variable */
struct dfv_struct
{
	long v;		/* current value */
	long min;	/* v >= min */
	long max;	/* v <= max */
};
typedef struct dfv_struct dfv_t;



/* artefact in graph */

#define AIG_POINT_MAX 5
#define AIG_DFV_MAX 3
struct aig_struct
{
	int eig_src;	/* element index (tig_list) source tool*/
	int dir_src;	/* 0 right edge, 1 bottom edge, 2 left edge, 3 top edge */
	
	int eig_dest; 	/* element index (tig_list) destination tool */
	int dir_dest;	/* 0 right edge, 1 bottom edge, 2 left edge, 3 top edge */
	
	int dfv_cnt;				/* degree of freedom for this path, denotes also the number of elements in dfv_list */
	dfv_t dfv_list[AIG_DFV_MAX];	/* independent variables and their ranges */
	int dfv_ref_list[AIG_POINT_MAX*2]; /* -1 if the value in point_val_list is valid. otherwise index into dfv_list */

	int point_val_cnt;	/* number of points, between 0 and AIG_POINT_MAX-1 */
	long point_val_list[AIG_POINT_MAX*2]; /* even: x, odd: y... not required???? */
	
};

/* the tool chain graph */
struct tcg_struct
{
	ps_t *tig_list;
	
	ps_t *aig_list;
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
/* "element in graph" procedures */

long tig_GetWidth(tig_t *tig);
long tig_GetHeight(tig_t *tig);


/*========================================*/
/* tcg procedures */

#define TCG_CONNECT_GRID_SIZE 6


#define tcg_GetTIgCnt(tcg) ps_Cnt((tcg)->tig_list)
#define tcg_WhileTig(tcg, idx) ps_WhileLoop((tcg)->tig_list, (idx))
#define tcg_GetTig(tcg, idx) ((tig_t *)ps_Get((tcg)->tig_list, (idx)))
#define tcg_SetTig(tcg, idx, tig) (ps_Set((tcg)->tig_list, (idx), (void *)(tig)))

#define tcg_GetAigCnt(tcg) ps_Cnt((tcg)->aig_list)
#define tcg_WhileAig(tcg, idx) ps_WhileLoop((tcg)->aig_list, (idx))
#define tcg_GetAig(tcg, idx) ((aig_t *)ps_Get((tcg)->aig_list, (idx)))
#define tcg_SetAig(tcg, idx, aig) (ps_Set((tcg)->aig_list, (idx), (void *)(aig)))

void tgc_CalculateDimension(tcg_t *tcg);
long tgc_GetGraphXFromView(tcg_t *tcg, double x);
long tgc_GetGraphYFromView(tcg_t *tcg, double y);
double tgc_GetViewXFromGraph(tcg_t *tcg, long x);
double tgc_GetViewYFromGraph(tcg_t *tcg, long y);

tcg_t *tcg_Open(void);
void tcg_Close(tcg_t *tcg);
void tcg_DeleteTig(tcg_t *tcg, int idx);
int tcg_AddTig(tcg_t *tcg, const char *name, long x, long y);
int tcg_AddAig(tcg_t *tcg, int eig_src, int dir_src, int eig_dest, int dir_dest);
void tcg_CalculateAigPath(tcg_t *tcg, int idx);
void tcg_ShowAigPoints(tcg_t *tcg, int aig_idx);

int tcg_CatchElement(tcg_t *tcg, double x, double y);
int tcg_IsCatched(tcg_t *tcg, int idx);
int tcg_IsSelected(tcg_t *tcg, int idx);
int tcg_SendEventWithGraphPosition(tcg_t *tcg, int event, long x, long y);
int tcg_SendEventWithViewPosition(tcg_t *tcg, int event, double x, double y);


/* tcg_connect.c */
int tcg_GetConnectCnt(tcg_t *tcg, int idx, int dir);
long tcg_GetConnectDeltaPos(tcg_t *tcg, int idx, int dir, int pos);
long tcg_GetConnectPosX(tcg_t *tcg, int idx, int dir, int pos);
long tcg_GetConnectPosY(tcg_t *tcg, int idx, int dir, int pos);


