/*

	tce_draw.c
	
	Tool Chain Editor, drawing procedures
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
	
	This file has only one function: 
		void tcg_RedrawAll(tcg_t *tcg, cairo_t *c)

*/


#include <gtk/gtk.h>
#include "tcg.h"
#include <string.h>
#include <stdio.h>

#define TCG_RECT_STYLE_CATCH_AREA 0
#define TCG_RECT_STYLE_TIG 1
#define TCG_RECT_STYLE_TIG_CONNECTOR 2
#define TCG_RECT_STYLE_SEG 3


static void tcg_DrawRect(tcg_t *tcg, rect_t *r, int style, int is_selected, int is_catched, cairo_t *c)
{
	double x0, x1, y0, y1;
	x0 =  tgc_GetViewXFromGraph(tcg, r->x0);
	x1 =  tgc_GetViewXFromGraph(tcg, r->x1);
	y0 =  tgc_GetViewYFromGraph(tcg, r->y0);
	y1 =  tgc_GetViewYFromGraph(tcg, r->y1);
	

	if ( style == TCG_RECT_STYLE_CATCH_AREA )
	{
		cairo_set_source_rgba (c, 1.0, 0.0, 0.0, 0.5);
		cairo_set_line_width (c, 3);
		cairo_new_path(c);
		cairo_rectangle(c, x0, y0, x1-x0, y1-y0);
		cairo_stroke (c);	
	}
	else if ( style == TCG_RECT_STYLE_TIG )
	{
		cairo_set_source_rgb (c, 0.7, 0.7, 0.7);
		cairo_rectangle (c, x0+1.0, y0+1.0, x1-x0-2.0, y1-y0-2.0);
		cairo_fill(c);
		if ( is_selected )
		{
			cairo_set_source_rgb (c, 1.0, 0.0, 0.0);
		}
		else
		{
			cairo_set_source_rgb (c, 0, 0, 0);
		}
		if ( is_catched )
		{
			cairo_set_line_width (c, 3* tcg->tcgv->zoom);
		}
		else
		{
			cairo_set_line_width (c, 1* tcg->tcgv->zoom);			
		}
		cairo_rectangle (c, x0, y0, x1-x0, y1-y0);
		cairo_stroke (c);		
	}
	else if ( style == TCG_RECT_STYLE_TIG_CONNECTOR )
	{
		cairo_set_source_rgb (c, 0.7, 0.7, 0.7);
		if ( is_selected )
		{
			cairo_set_source_rgb (c, 1.0, 0.0, 0.0);
		}
		else
		{
			cairo_set_source_rgb (c, 0, 0, 0);
		}
		if ( is_catched )
		{
			cairo_set_line_width (c, 3* tcg->tcgv->zoom);
		}
		else
		{
			cairo_set_line_width (c, 1* tcg->tcgv->zoom);			
		}
		cairo_rectangle (c, x0, y0, x1-x0, y1-y0);
		cairo_stroke (c);				
	}
	else if ( style == TCG_RECT_STYLE_SEG )
	{
		cairo_set_source_rgb (c, 0.7, 0.7, 0.7);
		if ( is_selected )
		{
			cairo_set_source_rgb (c, 1.0, 0.0, 0.0);
		}
		else
		{
			cairo_set_source_rgb (c, 0, 0, 0);
		}
		if ( is_catched )
		{
			cairo_set_line_width (c, 3* tcg->tcgv->zoom);
		}
		else
		{
			cairo_set_line_width (c, 1* tcg->tcgv->zoom);			
		}
		cairo_rectangle (c, x0, y0, x1-x0, y1-y0);
		//cairo_fill(c);
		cairo_stroke (c);				
	}
	
	
}



void tcg_DrawTig(tcg_t *tcg, int idx, cairo_t *c)
{
	tig_t *tig;
	double x0, x1, y0, y1;
	rect_t r;
	
	cairo_text_extents_t extents;

	tig = tcg_GetTig(tcg, idx);

	tcg_DrawRect(tcg, 
		&(tig->area), 
		TCG_RECT_STYLE_TIG, 
		tcg_IsTigSelected(tcg, idx),  
		tcg_IsTigCatched(tcg, idx), 
		c);
	
	if ( tcg_GetCatchedConnectRect(tcg, idx, &r) != 0 )
	{
		tcg_DrawRect(tcg, 
			&r, 
			TCG_RECT_STYLE_TIG_CONNECTOR, 
			tcg_IsTigSelected(tcg, idx),  
			tcg_IsTigCatched(tcg, idx), 
			c);
	}
	

	
	x0 =  tgc_GetViewXFromGraph(tcg, tig->area.x0);
	x1 =  tgc_GetViewXFromGraph(tcg, tig->area.x1);
	y0 =  tgc_GetViewYFromGraph(tcg, tig->area.y0);
	y1 =  tgc_GetViewYFromGraph(tcg, tig->area.y1);
	cairo_new_path(c);
	cairo_select_font_face (c, "sans-serif",  CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (c, 10* tcg->tcgv->zoom);
	cairo_set_source_rgb (c, 0, 0, 0);
	cairo_text_extents (c, "Test", &extents);
	
	/* center text within the area */
	cairo_move_to(c, x0 + ((x1-x0)-extents.width)/2 /* - extents.x_bearing */ , y0 + ((y1-y0)-extents.height)/2 - extents.y_bearing );
	cairo_show_text (c, "Test");
	
}

void tcg_DrawAig(tcg_t *tcg, int aig_idx, cairo_t *c)
{
	int j, cnt;
	rect_t r;
	
	
	cnt = tcg_GetAigSegCnt(tcg, aig_idx);
	for( j = 0; j < cnt; j++)
	{
		tcg_GetAigSegRect(tcg, aig_idx, j, &r);
		
		tcg_DrawRect(tcg, 
			&r, 
			TCG_RECT_STYLE_SEG, 
			/* selected */ tcg_IsAigSegSelected(tcg, aig_idx, j), 
			/* catched */ tcg_IsAigSegCatched(tcg, aig_idx, j), 
			c);
	}
	
}

void tcg_RedrawAll(tcg_t *tcg, cairo_t *c)
{
	int i;
	
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tcg_DrawTig(tcg, i, c);
	}
	
	i = -1;
	while( tcg_WhileAig(tcg, &i)  )
	{
		tcg_DrawAig(tcg, i, c);
	}
	
	if ( tcg_IsCatchAreaVisible(tcg) )
	{
		
		tcg_DrawRect(tcg, 
			&(tcg->catch_area), 
			TCG_RECT_STYLE_CATCH_AREA, 
			0,  
			0, 
			c);
	}
}

