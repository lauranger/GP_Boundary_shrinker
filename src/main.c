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

#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "interface.h"
#include "render.h"

#include "plugin-intl.h"


/*  Constants  */

#define PROCEDURE_NAME   "gimp_thiner"
#define ALIEN_PROCEDURE_NAME   "n_pixels_by_cmap_entries"

#define DATA_KEY_VALS    "boundary_shrinker_plugin"
#define DATA_KEY_UI_VALS "boundary_shrinker_plugin_ui"

#define PARASITE_KEY     "boundary-shrinker-plugin-options"


/*  Local function prototypes  */

static void   query (void);
static void   run   (const gchar      *name,
					 gint              nparams,
					 const GimpParam  *param,
					 gint             *nreturn_vals,
					 GimpParam       **return_vals);


/*  Local variables  */

const PlugInVals default_vals =
{
	{0,0,0,.0},
	-1, 2, {1,1,1,.0},
	-1, TRUE
};

const PlugInImageVals default_image_vals =
{
	0
};

const PlugInDrawableVals default_drawable_vals =
{
	0
};

static PlugInVals         vals;
static PlugInImageVals    image_vals;
static PlugInDrawableVals drawable_vals;


GimpPlugInInfo PLUG_IN_INFO =
{
	NULL,  /* init_proc  */
	NULL,  /* quit_proc  */
	query, /* query_proc */
	run,   /* run_proc   */
};

static guint closest_entry_in_cmap(GimpRGB given_color, gint32 image_ID);

MAIN ()

static void
query (void)
{
	gchar *help_path;
	gchar *help_uri;

	static GimpParamDef bs_args[] =
	{
		{ GIMP_PDB_INT32,    "run_mode",   "Interactive, non-interactive"    },
		{ GIMP_PDB_IMAGE,    "image",      "Input image"                     },
		{ GIMP_PDB_DRAWABLE, "drawable",   "Input drawable"                  },
		{ GIMP_PDB_COLOR,    "skel_color", "Boundary color"                  },
		{ GIMP_PDB_INT32,    "nb_grow_passes", "Number of grow passes [0-2]" },
		{ GIMP_PDB_COLOR,    "color_no_fill",  "Secondary color, not to fill"},
	};

	static GimpParamDef cc_in_args[] =
	{
		{ GIMP_PDB_INT32,    "run_mode",   "Interactive, non-interactive"    },
		{ GIMP_PDB_IMAGE,    "image",      "Input image"                     },
		{ GIMP_PDB_DRAWABLE, "drawable",   "Input drawable"                  },
	};
	static GimpParamDef cc_out_args[] =
	{
		{ GIMP_PDB_STATUS,    "status","Status" },
		{ GIMP_PDB_INT32,     "nb_used_colors","Number of colors with at least one pixel" },
		//{ GIMP_PDB_INT32,     "next_array_size","Number of entries in next (256)" },
		{ GIMP_PDB_INT32ARRAY,"n_pix_by_cmap_entries","Number of pixels by colormap entry"},
	};

	gimp_plugin_domain_register (PLUGIN_NAME, LOCALEDIR);

	help_path = g_build_filename (DATADIR, "help", NULL);
	help_uri = g_filename_to_uri (help_path, NULL, NULL);
	g_free (help_path);

	gimp_plugin_help_register ("http://gna.org/boundary_shrinker/help",
							   help_uri);

	gimp_install_procedure (PROCEDURE_NAME,
							"Shrink boundaries to one pixel thick",
							"No help yet",
							"Laurent G. <lauranger@gna.org>",
							"Laurent G. <lauranger@gna.org>",
							"2007-2007",
							N_("Boundary shrinker"),
							"INDEXED*",
							GIMP_PLUGIN,
							G_N_ELEMENTS (bs_args), 0,
							bs_args, NULL);

	gimp_install_procedure (ALIEN_PROCEDURE_NAME,
							"Counts number of pixel for each colormap entries",
							"No help yet",
							"Laurent G. <lauranger@gna.org>",
							"Laurent G. <lauranger@gna.org>",
							"2007-2007",
							N_("Indexed Colors Usage"),
							"INDEXED",
							GIMP_PLUGIN,
							G_N_ELEMENTS (cc_in_args), G_N_ELEMENTS (cc_out_args),
							cc_in_args, cc_out_args);

	gimp_plugin_menu_register (PROCEDURE_NAME, "<Image>/Filters/Misc/");
}

static void run (const gchar      *name,
				 gint              n_params,
				 const GimpParam  *param,
				 gint             *nreturn_vals,
				 GimpParam       **return_vals)
{
	static GimpParam   values[4];
	GimpDrawable      *drawable;
	gint32             image_ID;
	GimpRunMode        run_mode;
	GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

	*return_vals  = values;

	/*  Initialize i18n support  */
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
	textdomain (GETTEXT_PACKAGE);

	run_mode = param[0].data.d_int32;
	image_ID = param[1].data.d_int32;
	drawable = gimp_drawable_get(param[2].data.d_drawable);

	/*  Initialize with default values  */
	vals          = default_vals;
	image_vals    = default_image_vals;
	drawable_vals = default_drawable_vals;

	if (strcmp (name, PROCEDURE_NAME) == 0)
	{
		*nreturn_vals = 1;
		switch (run_mode)
		{
			case GIMP_RUN_NONINTERACTIVE:
				if (n_params != 7)
				{
					status = GIMP_PDB_CALLING_ERROR;
				}
				else
				{
					vals.skel_color      = param[3].data.d_color;
					vals.nb_passes	     = param[4].data.d_int32;
					vals.color_no_fill   = param[5].data.d_color;
					vals.skel_idx = closest_entry_in_cmap(vals.skel_color, image_ID);
					vals.color_no_fill_idx = closest_entry_in_cmap(vals.color_no_fill, image_ID);
				}
				break;

			case GIMP_RUN_INTERACTIVE:
				/*  Possibly retrieve data  */
				gimp_get_data (DATA_KEY_VALS,    &vals);

				if (! dialog (image_ID, drawable,
							  &vals, &image_vals, &drawable_vals))
				{
					status = GIMP_PDB_CANCEL;
				}
				else
				{
					vals.skel_idx = closest_entry_in_cmap(vals.skel_color, image_ID);
					vals.color_no_fill_idx = closest_entry_in_cmap(vals.color_no_fill, image_ID);
				}
				break;

			case GIMP_RUN_WITH_LAST_VALS:
				/*  Possibly retrieve data  */
				gimp_get_data (DATA_KEY_VALS, &vals);
				vals.skel_idx = closest_entry_in_cmap(vals.skel_color, image_ID);
				vals.color_no_fill_idx = closest_entry_in_cmap(vals.color_no_fill, image_ID);

				break;

			default:
				break;
		}

		if (status == GIMP_PDB_SUCCESS)
		{
			if ( vals.select_changed )
				render_full(image_ID, drawable, &vals, &image_vals, &drawable_vals);
			else
				render_tiled(image_ID, drawable, &vals, &image_vals, &drawable_vals);

			if (run_mode != GIMP_RUN_NONINTERACTIVE)
				gimp_displays_flush();

			if (run_mode == GIMP_RUN_INTERACTIVE)
				gimp_set_data(DATA_KEY_VALS,    &vals,    sizeof (vals));

			//done in render.c
			//gimp_drawable_detach(drawable);
		}
	}
	else if (strcmp (name, ALIEN_PROCEDURE_NAME) == 0)
	{
		static gint32 colors_usage[256];
		//gint32 *colors_usage = g_new(gint32, 256);
		*nreturn_vals = 4;
		values[1].type = GIMP_PDB_INT32;
		values[2].type = GIMP_PDB_INT32;
		values[3].type = GIMP_PDB_INT32ARRAY;

		values[2].data.d_int32 = 256L;
		values[3].data.d_int32array = colors_usage;
		gint32 nb_used = 0;

		count_pixels(image_ID, drawable, &nb_used, colors_usage);

		values[1].data.d_int32 = nb_used;
	}
	else
	{
		status = GIMP_PDB_CALLING_ERROR;
	}

	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;
}
/*
image = gimp.image_list()[0]
drawable = pdb.gimp_image_get_active_drawable(image)
status, nb_used_colors, next_array_size, n_pix_by_cmap_entries = pdb.n_pixels_by_cmap_entries(image, drawable)
status, nb_used_colors, next_array_size, n_pix_by_cmap_entries
*/

static guint closest_entry_in_cmap(GimpRGB given_color, gint32 image_ID)
{
	gint l_num;
	guint ret = -1;
	guchar *l_colors = gimp_image_get_colormap(image_ID, &l_num);
	if ( l_num > 0 )
	{
		gdouble min_distance = 4.0;
		guint triplet_idx;
		for ( triplet_idx = 0 ; triplet_idx < l_num ; triplet_idx++ )
		{
			gdouble r_diff = (gdouble)l_colors[3*triplet_idx + 0] / 255.0 -given_color.r;
			gdouble g_diff = (gdouble)l_colors[3*triplet_idx + 1] / 255.0 -given_color.g;
			gdouble b_diff = (gdouble)l_colors[3*triplet_idx + 2] / 255.0 -given_color.b;
			gdouble dist = r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;
			if ( dist < min_distance )
			{
				min_distance = dist;
				ret = triplet_idx;
			}
		}
	}
	return ret;
}

