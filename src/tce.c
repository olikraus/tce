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


void tcg_RedrawAll(tcg_t *tcg, cairo_t *c); 	/* tce_draw.c */


GtkBuilder *builder; 
GtkLabel *zoom_value;
GtkScrollbar *da_h_scrollbar;
GtkDrawingArea *da;
GtkAboutDialog *aboutdialog;
GtkDialog *tool_dialog;
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


/*==============================================================*/
/* signal handler */


G_MODULE_EXPORT void on_da_popup_delete_path_activate (GtkMenuItem *menuitem, gpointer d)
{
	tcg_t *tcg = (tcg_t *)d;
	tcg_SendEventWithViewPosition(tcg, TCG_EVENT_DELETE_PATH, da_popup_menu_x, da_popup_menu_y);
	puts("on_da_popup_delete_path_activate");
	
	gtk_widget_queue_draw (GTK_WIDGET(da));	
}

G_MODULE_EXPORT void on_da_popup_add_segment_activate (GtkMenuItem *menuitem, gpointer d)
{
	tcg_t *tcg = (tcg_t *)d;
	tcg_SendEventWithViewPosition(tcg, TCG_EVENT_INSERT_SEGMENT, da_popup_menu_x, da_popup_menu_y);
	
	gtk_widget_queue_draw (GTK_WIDGET(da));
}

G_MODULE_EXPORT void on_event_tool_dialog_cancel(GtkMenuItem *menuitem, gpointer d)
{
	gtk_dialog_response (tool_dialog, 0);
	gtk_widget_hide(GTK_WIDGET(tool_dialog));
	puts("on_event_tool_dialog_cancel");
}

G_MODULE_EXPORT void on_event_tool_dialog_ok(GtkMenuItem *menuitem, gpointer d)
{
	gtk_dialog_response (tool_dialog, 1);	
	gtk_widget_hide(GTK_WIDGET(tool_dialog));
	puts("on_event_tool_dialog_ok");
}

G_MODULE_EXPORT void on_da_popup_edit_tool_activate (GtkMenuItem *menuitem, gpointer d)
{
	int response;
	tcg_t *tcg = (tcg_t *)d;
	puts("on_da_popup_edit_tool_activate");
	gtk_widget_show_all (GTK_WINDOW(tool_dialog));                

	response = gtk_dialog_run(tool_dialog);
	gtk_widget_queue_draw (GTK_WIDGET(da));
}

G_MODULE_EXPORT void on_da_popup_new_tool_activate (GtkMenuItem *menuitem, gpointer d)
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
	
	//printf("on_da_motion_notify_event: %f %f %u state=%d\n", x, y, time, tcg->state);
	

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

	//printf("on_da_draw: %u %u\n", width, height);
	
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
	//builder = gtk_builder_new();
	//gtk_builder_add_from_file(builder, "tce.glade", NULL);
	
	window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	da_h_scrollbar = GTK_SCROLLBAR(gtk_builder_get_object (builder, "da_h_scrollbar"));
	zoom_value = GTK_LABEL (gtk_builder_get_object (builder, "zoom_value"));
	da = GTK_DRAWING_AREA (gtk_builder_get_object (builder, "da"));
	aboutdialog = GTK_ABOUT_DIALOG (gtk_builder_get_object (builder, "aboutdialog"));
	da_popup = GTK_MENU(gtk_builder_get_object (builder, "da_popup"));
	tool_dialog = GTK_DIALOG(gtk_builder_get_object (builder, "tool_dialog"));
		gtk_window_set_transient_for (GTK_WINDOW(tool_dialog), GTK_WINDOW(window));
	
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
