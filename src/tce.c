/*

	tce.c
	
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


#include <gtk/gtk.h>
#include "tcg.h"
#include <string.h>
#include <stdio.h>


GtkBuilder *builder; 
GtkLabel *zoom_value;
GtkScrollbar *da_h_scrollbar;
GtkDrawingArea *da;
GtkAboutDialog *aboutdialog;
GtkMenu *da_popup;
double da_popup_menu_x;
double da_popup_menu_y;


void box(GtkWidget *w, gdouble x, gdouble y)
{
	cairo_t *c;
	c = gdk_cairo_create(gtk_widget_get_window (w));
	cairo_set_source_rgb (c, 0, 0, 255);
	cairo_set_line_width (c, 1);
	cairo_rectangle (c, x, y, 1, 1);
	cairo_stroke (c);
	cairo_destroy (c);
}

/*==============================================================*/

void tcg_UpdateScrollbar(tcg_t *tcg)
{
	GtkAdjustment *a;

	guint da_width, da_height;	/* dimensions of the drawing area */
	
	long graph_lower;
	long graph_upper;
	long graph_position;
	double graph_view_size;
	
	gdouble value;
	gdouble lower;
	gdouble upper;
	gdouble step_increment;
	gdouble page_increment;
	gdouble page_size;

	a = gtk_range_get_adjustment (GTK_RANGE(da_h_scrollbar));
	
	/* The value of the scrollbar can be between lower and upper-page_size */
	
	da_width = gtk_widget_get_allocated_width (GTK_WIDGET(da));
	graph_position = tcg->tcgv->x;
	graph_lower = tcg->graph_dimension.x0;
	graph_upper = tcg->graph_dimension.x1;
	graph_view_size = (double)(da_width / tcg->tcgv->zoom);
	
	value = graph_position;
	lower = graph_lower;
	upper = graph_upper;
	step_increment = graph_view_size * 0.01;
	page_increment = graph_view_size * 0.9;
	page_size = graph_view_size;
	
	printf("scrollbar %lf .. %lf/%lf .. %lf\n", lower, value, page_size, upper);
	
	gtk_adjustment_configure (a,
                          value,
                          lower,
                          upper,
                          step_increment,
                          page_increment,	
                          page_size);
			  
	da_height = gtk_widget_get_allocated_height (GTK_WIDGET(da));	
}



void tcg_UpdateZoomValue(tcg_t *tcg)
{
	char s[128];
	sprintf(s, "%.2f%%", tcg->tcgv->zoom*100.0);
	gtk_label_set_label (zoom_value, s);
}

void tcg_DrawTig(tcg_t *tcg, int idx, cairo_t *c)
{
	tig_t *tig;
	double x0, x1, y0, y1;
	double x, y;
	int dir, pos, cnt;
	
	cairo_text_extents_t extents;

	
	
	tig = tcg_GetTig(tcg, idx);

	
	x0 =  tgc_GetViewXFromGraph(tcg, tig->area.x0);
	x1 =  tgc_GetViewXFromGraph(tcg, tig->area.x1);
	y0 =  tgc_GetViewYFromGraph(tcg, tig->area.y0);
	y1 =  tgc_GetViewYFromGraph(tcg, tig->area.y1);

	// printf("tcg_DrawTig: dx=%ld dy=%ld    x0=%ld x1=%ld y0=%ld y1=%ld    x0=%lf x1=%lf y0=%lf y1=%lf\n", tcg->tcgv->x, tcg->tcgv->y, tig->area.x0, tig->area.x1, tig->area.y0, tig->area.y1, x0, x1, y0, y1);

	cairo_set_source_rgb (c, 0.7, 0.7, 0.7);
	cairo_rectangle (c, x0+1.0, y0+1.0, x1-x0-2.0, y1-y0-2.0);
	cairo_fill(c);
	
	if ( tcg_IsTigSelected(tcg, idx) )
	{
		cairo_set_source_rgb (c, 1.0, 0.0, 0.0);
	}
	else
	{
		cairo_set_source_rgb (c, 0, 0, 0);
	}
	
	
	if ( tcg_IsCatched(tcg, idx) )
	{
		cairo_set_line_width (c, 3* tcg->tcgv->zoom);
	}
	else
	{
		cairo_set_line_width (c, 1* tcg->tcgv->zoom);
	}
	cairo_rectangle (c, x0, y0, x1-x0, y1-y0);
	cairo_stroke (c);
	

	/*
	for( dir = 0; dir < 4; dir++ )
	{
		cnt = tcg_GetConnectCnt(tcg, idx, dir);
		printf("cnt = %d\n", cnt);
		for ( pos = 0; pos < cnt; pos++ )
		{
			x = tgc_GetViewXFromGraph(tcg, tcg_GetConnectPosX(tcg, idx, dir, pos));
			y = tgc_GetViewYFromGraph(tcg, tcg_GetConnectPosY(tcg, idx, dir, pos));
			//printf("cnt = %d   x=%lf y=%lf\n", cnt, x, y);
			
			cairo_rectangle (c, x-1, y-1, 3, 3);
			cairo_stroke (c);
		}
	}
	*/
	
	if ( tcg_GetCatchedConnect(tcg, idx, &dir, &pos) )
	{
		x = tgc_GetViewXFromGraph(tcg, tcg_GetConnectPosX(tcg, idx, dir, pos));
		y = tgc_GetViewYFromGraph(tcg, tcg_GetConnectPosY(tcg, idx, dir, pos));
		cairo_rectangle (c, x-TCG_CONNECT_HALF_WIDTH, y-TCG_CONNECT_HALF_WIDTH, 2*TCG_CONNECT_HALF_WIDTH+1, 2*TCG_CONNECT_HALF_WIDTH+1);
		cairo_stroke (c);
	}
	

	
	
	cairo_new_path(c);
	cairo_select_font_face (c, "sans-serif",  CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (c, 10* tcg->tcgv->zoom);
	cairo_set_source_rgb (c, 0, 0, 0);
	cairo_text_extents (c, "Test", &extents);
	
	/* center text within the area */
	cairo_move_to(c, x0 + ((x1-x0)-extents.width)/2 /* - extents.x_bearing */ , y0 + ((y1-y0)-extents.height)/2 - extents.y_bearing );
	cairo_show_text (c, "Test");
	
	
}

void tcg_RedrawAll(tcg_t *tcg, cairo_t *c)
{
	int i;
	
	i = -1;
	while( tcg_WhileTig(tcg, &i) )
	{
		tcg_DrawTig(tcg, i, c);
	}
	
	/* intermediate line drawing, will be removed */
	i = -1;
	while( tcg_WhileAig(tcg, &i)  )
	{
		int j, cnt;
		cnt = tcg_GetAigPointCnt(tcg, i);
		
		cairo_set_source_rgba (c, 0.0, 0.0, 1.0, 1.0);
		cairo_set_line_width (c, 1);
		cairo_new_path(c);
		cairo_move_to(c, tgc_GetViewXFromGraph(tcg, tcg_GetAigPointX(tcg, i, 0)), tgc_GetViewYFromGraph(tcg, tcg_GetAigPointY(tcg, i, 0)));
		
		for( j = 1; j < cnt; j++ )
		{
			cairo_line_to(c, tgc_GetViewXFromGraph(tcg, tcg_GetAigPointX(tcg, i, j)), tgc_GetViewYFromGraph(tcg, tcg_GetAigPointY(tcg, i, j)));
		}
		cairo_stroke (c);	
	}
	
	if ( tcg_IsCatchAreaVisible(tcg) )
	{
		double x0, x1, y0, y1; 
		
		x0 = tgc_GetViewXFromGraph(tcg, tcg->catch_area.x0);
		x1 = tgc_GetViewXFromGraph(tcg, tcg->catch_area.x1);
		y0 = tgc_GetViewYFromGraph(tcg, tcg->catch_area.y0);
		y1 = tgc_GetViewYFromGraph(tcg, tcg->catch_area.y1);
		
		// printf("catch area: x0=%lf x1=%lf y0=%lf y1=%lf\n", x0, x1, y0, y1);

		
		cairo_set_source_rgba (c, 1.0, 0.0, 0.0, 0.5);
		cairo_set_line_width (c, 3);
		cairo_new_path(c);
		cairo_rectangle(c, x0, y0, x1-x0, y1-y0);
		cairo_stroke (c);	
		
	}
}

/*==============================================================*/
/* signal handler */


G_MODULE_EXPORT void on_da_popup_new_activate (GtkMenuItem *menuitem, gpointer d)
{
	tcg_t *tcg = (tcg_t *)d;
	tcg_AddTig(tcg, NULL,  tgc_GetGraphXFromView(tcg, da_popup_menu_x), tgc_GetGraphYFromView(tcg, da_popup_menu_y));
	gtk_widget_queue_draw (GTK_WIDGET(da));	
}

G_MODULE_EXPORT gboolean on_da_motion_notify_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
	guint32 time;
	gdouble x, y;
	tcg_t *tcg = (tcg_t *)d;
	
	time = e->motion.time;
	x = e->motion.x;
	y = e->motion.y;

	
	if ( tcg_SendEventWithViewPosition(tcg, TCG_EVENT_MOUSE_MOVE, x, y) )
	{
		gtk_widget_queue_draw (w);
	}
	
	printf("on_da_motion_notify_event: %f %f %u state=%d\n", x, y, time, tcg->state);
	

	box(w,x,y);
	
	
	return TRUE; /* signal that the event has been handled */
}

G_MODULE_EXPORT gboolean on_da_button_press_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
	gdouble x, y;
	guint state;	/* https://developer.gnome.org/gdk3/stable/gdk3-Windows.html#GdkModifierType */
	guint button;   /* https://developer.gnome.org/gdk3/stable/gdk3-Event-Structures.html#GdkEventButton */
	
	tcg_t *tcg = (tcg_t *)d;
	
	
	x = e->button.x;
	y = e->button.y;
	state = e->button.state;
	button = e->button.button;

	if ( button == 3 )
	{
		//gtk_widget_show (GTK_WIDGET(da_popup));
		da_popup_menu_x = x;
		da_popup_menu_y = y;
		
		gtk_menu_popup (da_popup,
			NULL,
			NULL,
			NULL,
			d,
			button,
			e->button.time);
		

	}
	else if ( button == 1 )
	{
		if ( state & GDK_SHIFT_MASK )
		{
			if ( tcg_SendEventWithViewPosition(tcg, TCG_EVENT_SHIFT_BUTTON_DOWN, x, y) )
			{
				gtk_widget_queue_draw (w);
			}
		}
		else
		{
			if ( tcg_SendEventWithViewPosition(tcg, TCG_EVENT_BUTTON_DOWN, x, y) )
			{
				gtk_widget_queue_draw (w);
			}
		}
	}
	
	printf("on_da_button_press_event: %f %f %d state=%d\n", x, y, state, tcg->state);
	
	
	return TRUE; /* signal that the event has been handled */
}

G_MODULE_EXPORT gboolean on_da_button_release_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
	/* Event: https://developer.gnome.org/gdk3/stable/gdk3-Event-Structures.html#GdkEventButton */
	
	
	gdouble x, y;
	guint state;	/* https://developer.gnome.org/gdk3/stable/gdk3-Windows.html#GdkModifierType */
	guint button;   /* https://developer.gnome.org/gdk3/stable/gdk3-Event-Structures.html#GdkEventButton */
	
	tcg_t *tcg = (tcg_t *)d;
	
	
	x = e->button.x;
	y = e->button.y;
	state = e->button.state;
	button = e->button.button;
	
	if ( button == 3 )
	{
		
		
		//tcg_AddTig(tcg, NULL,  tgc_GetGraphXFromView(tcg, x), tgc_GetGraphYFromView(tcg, y));
		//gtk_widget_queue_draw (w);	/* force redraw , maybe better use gtk_widget_queue_draw_area() */
	}
	else if ( button == 1 )
	{
		if ( tcg_SendEventWithViewPosition(tcg, TCG_EVENT_BUTTON_UP, x, y) )
		{
			gtk_widget_queue_draw (w);
		}
	}

	printf("on_da_button_release_event: %f %f %d state=%d\n", x, y, state, tcg->state);

	
	return TRUE; /* signal that the event has been handled */
}

/* redraw happens here and nowhere else */
G_MODULE_EXPORT void on_da_draw(GtkWidget *w, cairo_t *c, gpointer d)
{
	guint width, height;
	tcg_t *tcg = (tcg_t *)d;

	width = gtk_widget_get_allocated_width (w);
	height = gtk_widget_get_allocated_height (w);

	printf("on_da_draw: %u %u\n", width, height);
	
	tcg_RedrawAll(tcg, c);
}

/* catch mouse wheel events */
G_MODULE_EXPORT void on_da_scroll_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
	gdouble delta_y;
	tcg_t *tcg = (tcg_t *)d;
	/* https://developer.gnome.org/gdk3/stable/gdk3-Event-Structures.html#GdkEventScroll */

	delta_y = e->scroll.delta_y;	/* < 0.0 for up, > 0.0 for down */
	if ( delta_y > 0.5 )
	{
		tcg->tcgv->zoom /= 2.0;
	}
	else if ( delta_y < -0.5 )
	{
		tcg->tcgv->zoom *= 2.0;
	}
	printf("on_da_scroll_event: %f, new zoom: %f\n", delta_y, tcg->tcgv->zoom);
		
	tcg_UpdateZoomValue(tcg);
	tcg_UpdateScrollbar(tcg);
	gtk_widget_queue_draw (w);	/* force redraw */
}

/* slide moved events */
G_MODULE_EXPORT void on_da_h_scrollbar_value_changed (GtkRange *r, gpointer  d)
{
	GtkAdjustment *a;
	
	double value;
	guint  da_width;
	double graph_view_size;
	
	tcg_t *tcg = (tcg_t *)d;
	
	value = gtk_range_get_value (r);
	
	da_width = gtk_widget_get_allocated_width (GTK_WIDGET(da));
	graph_view_size = (double)(da_width / tcg->tcgv->zoom);
	
	tcg->tcgv->x = value;
	
	a = gtk_range_get_adjustment (r);
	
	printf("on_da_h_scrollbar_value_changed: x=%ld, %lf .. %lf/%lf .. %lf\n", tcg->tcgv->x,
		gtk_adjustment_get_lower (a), gtk_adjustment_get_value (a), gtk_adjustment_get_page_size (a), gtk_adjustment_get_upper (a));
	
	gtk_widget_queue_draw (GTK_WIDGET(da));	/* force redraw */
}

/* drawing area size has changed */
G_MODULE_EXPORT void on_da_configure_event(GtkWidget *w, GdkEvent  *e, gpointer d)
{
	tcg_t *tcg = (tcg_t *)d;

	/*
	called for any resize of the window
	e->configure.width;
	e->configure.height;
	the drawing area already has the target size, so just update the scrollbar
	*/
	
	tcg_UpdateScrollbar(tcg);
	
	/* draw event is also sent automatically */
}

G_MODULE_EXPORT void aboutdialog_activate (GtkMenuItem *w, gpointer d)
{
	gtk_widget_show (GTK_WIDGET(aboutdialog));
	gtk_dialog_run (GTK_DIALOG(aboutdialog));
	gtk_widget_hide (GTK_WIDGET(aboutdialog));
}

G_MODULE_EXPORT void on_window_destroy (GtkWidget *w, gpointer d)
{
    gtk_main_quit ();
}

/*==============================================================*/


int main (int argc, char *argv[])
{
	tcg_t *tcg;
	
	GtkWidget       		*window;

	gtk_init (&argc, &argv);
	
	tcg = tcg_Open();
	{
		int i1, i2;
		i1 = tcg_AddTig(tcg, "x", 20, 20);
		i2 = tcg_AddTig(tcg, "x", 70, 50);
	
		tcg_AddAig(tcg, i1, 0, 0, i2, 0, 0);
		//tcg_AddAig(tcg, i1, 1, 0, i2, 1, 0);
		//tcg_AddAig(tcg, i1, 2, 0, i2, 2, 0);
		//tcg_AddAig(tcg, i1, 3, 0, i2, 3, 0);
	}


	/* https://developer.gnome.org/gtk3/stable/GtkBuilder.html#gtk-builder-new-from-file */
	builder = gtk_builder_new_from_file ("tce.glade");
	
	window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	da_h_scrollbar = GTK_SCROLLBAR(gtk_builder_get_object (builder, "da_h_scrollbar"));
	zoom_value = GTK_LABEL (gtk_builder_get_object (builder, "zoom_value"));
	da = GTK_DRAWING_AREA (gtk_builder_get_object (builder, "da"));
	aboutdialog = GTK_ABOUT_DIALOG (gtk_builder_get_object (builder, "aboutdialog"));
	da_popup = GTK_MENU(gtk_builder_get_object (builder, "da_popup"));
	
	/* is there a description of the event masks??? */
	// this can be set in glade
	// gtk_widget_add_events (GTK_WIDGET (da), GDK_ALL_EVENTS_MASK);
	
	/* https://developer.gnome.org/gtk3/stable/GtkBuilder.html#gtk-builder-connect-signals */
	gtk_builder_connect_signals (builder, tcg);

	/* g_object_unref (G_OBJECT (builder)); required later for the about dialog */

	tcg_UpdateZoomValue(tcg);
	tcg_UpdateScrollbar(tcg);
	gtk_widget_show_all (window);                
	gtk_main ();

	tcg_Close(tcg);

	return 0;
}