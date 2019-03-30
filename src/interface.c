/* GIMP Plug-in BoundaryShrinker
 * Copyright (C) 2007  Laurent G <lauranger@gna.org> (the "Author").
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "interface.h"

#include "plugin-intl.h"


/*  Constants  */

#define COLORBUTTONWIDTH  30
#define COLORBUTTONHEIGHT 20

typedef struct
{
	GtkWidget *general_color_button;
	GtkWidget *sec_color_button;
	GtkWidget *color_lbl;
	GtkWidget *sec_color_lbl;
	GtkWidget *what_cbb;
} GPBoundaryShrkDlg;

/*  Local function prototypes  */

static void mode_changed(GtkComboBox *widget, gpointer user_data);

/*  Public functions  */

gboolean dialog (gint32              image_ID,
				 GimpDrawable       *drawable,
				 PlugInVals         *vals,
				 PlugInImageVals    *image_vals,
				 PlugInDrawableVals *drawable_vals)
{
	GtkWidget *dlg;
	GtkWidget *main_vbox, *color_hbox, *sec_color_hbox;
	GtkWidget *general_color_button, *sec_color_button;
	GtkWidget *color_lbl, *what_cbb, *sec_color_lbl;
	GtkWidget *selec_chb;

	gboolean   run = FALSE;

	gimp_ui_init (PLUGIN_NAME, TRUE);

	dlg = gimp_dialog_new (_("Boundary shrinker"), PLUGIN_NAME,
			NULL, 0,
			gimp_standard_help_func, "boundary-shrinker-plugin",

			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,

			NULL);

	main_vbox = gtk_vbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dlg)->vbox), main_vbox);

	color_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), color_hbox, FALSE, FALSE, 0);
	gtk_widget_show(color_hbox);

	general_color_button = gimp_color_button_new(_("Color"),
												 COLORBUTTONWIDTH,
												 COLORBUTTONHEIGHT,
												 &vals->skel_color,
												 GIMP_COLOR_AREA_FLAT);
	g_signal_connect(general_color_button, "color-changed",
					 G_CALLBACK(gimp_color_button_get_color),
					 &vals->skel_color);
	gtk_box_pack_start(GTK_BOX(color_hbox), general_color_button, FALSE, FALSE, 0);
	gtk_widget_show(general_color_button);

	color_lbl = gtk_label_new(_("Frontier color"));
	gtk_box_pack_start(GTK_BOX(color_hbox), color_lbl, FALSE, FALSE, 0);
	gtk_widget_show(color_lbl);

	what_cbb = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(what_cbb), _("Shrink only"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(what_cbb), _("Grow only non conflict"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(what_cbb), _("Grow in two passes"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(what_cbb), vals->nb_passes);
	g_signal_connect(what_cbb, "changed",
					 G_CALLBACK(gtk_combo_box_get_active),
					 &vals->nb_passes);
	gtk_box_pack_start(GTK_BOX(color_hbox), what_cbb, FALSE, FALSE, 0);
	gtk_widget_show(what_cbb);

	sec_color_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), sec_color_hbox, FALSE, FALSE, 0);
	gtk_widget_show(sec_color_hbox);

	sec_color_button = gimp_color_button_new(_("Rivers color"),
											 COLORBUTTONWIDTH,
											 COLORBUTTONHEIGHT,
											 &vals->color_no_fill,
											 GIMP_COLOR_AREA_FLAT);
	g_signal_connect(sec_color_button, "color-changed",
					 G_CALLBACK(gimp_color_button_get_color),
					 &vals->color_no_fill);
	gtk_box_pack_start(GTK_BOX(sec_color_hbox), sec_color_button, FALSE, FALSE, 0);
	gtk_widget_show(sec_color_button);

	sec_color_lbl = gtk_label_new(_("Rivers color"));
	gtk_box_pack_start(GTK_BOX(sec_color_hbox), sec_color_lbl, FALSE, FALSE, 0);
	gtk_widget_show(sec_color_lbl);

	selec_chb = gtk_check_button_new_with_mnemonic(_("_Select changes"));
	gtk_box_pack_end(GTK_BOX(sec_color_hbox), selec_chb, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(selec_chb), vals->select_changed);
	gtk_widget_show(selec_chb);

	g_signal_connect(what_cbb, "changed",
					 G_CALLBACK(mode_changed),
					 sec_color_button);
	/*  Show the main containers  */

	gtk_widget_show (main_vbox);
	gtk_widget_show (dlg);

	run = (gimp_dialog_run (GIMP_DIALOG (dlg)) == GTK_RESPONSE_OK);
	if ( run )
	{
		vals->nb_passes = gtk_combo_box_get_active(GTK_COMBO_BOX(what_cbb));
		vals->select_changed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(selec_chb));
	}

	gtk_widget_destroy (dlg);

	return run;
}


/*  Private functions  */

static void mode_changed(GtkComboBox *widget, gpointer user_data)
{
	GtkWidget *button = GTK_WIDGET(user_data);
	gtk_widget_set_sensitive(button, gtk_combo_box_get_active(widget) > 0 );
}

