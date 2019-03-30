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

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"

#include "plugin-intl.h"

static int sai_thiner[256] = {
	1, 2, 1, 3, 1, 0, 1, 8, 1, 0, 1, 8, 1, 0, 1, 8,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 8, 1, 0, 1, 8,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 8, 8, 8, 0, 8, 8,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 8, 8, 8, 0, 8, 8,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 8, 8, 8, 0, 8, 8,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 8, 8, 8, 0, 8, 8,
	1, 2, 7, 6, 0, 0, 4, 4, 0, 0, 6, 6, 0, 0, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6,
	2, 4, 4, 4, 0, 0, 4, 4, 2, 2, 0, 0, 2, 2, 0, 0,
	2, 2, 4, 4, 0, 0, 4, 4, 2, 2, 0, 0, 2, 2, 0, 0,
	2, 2, 4, 4, 0, 0, 4, 4, 0, 0, 6, 6, 0, 0, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6,
	2, 2, 4, 4, 0, 0, 4, 4, 2, 2, 0, 0, 2, 2, 0, 0,
	2, 2, 4, 4, 0, 0, 4, 4, 2, 2, 0, 0, 2, 2, 0, 0};

static int sai_grower[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 2, 2, 0, 0, 2, 0,
	0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 2, 0, 0, 2, 0,
	1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 2, 2, 0, 0, 2, 0,
	0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 2, 0, 0, 2, 0,
	0, 0, 2, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0};


typedef struct 
{
	GimpPixelRgn *rgn_orig;
	GimpPixelRgn *rgn_inout;
	guchar		 **in_rows;
	guchar		 *empty_row;
	guint		 x1;
	guint		 width;
	guint		 y1;
	guint		 height;
	GimpDrawable *drawable;
} gp_bs_tiled_data;

typedef struct 
{
	guchar		 **in_rows;
	guchar		 *empty_row;
	guchar		 *img_bytes;
	guchar		 *mask_bytes;
	guint		 x1;
	guint		 width;
	guint		 y1;
	guint		 height;
} gp_bs_full_data;

static guint thin_row(guchar      **in_rows,
					  guchar      *mask_row,
					  gint        width,
					  guchar      skel_idx,
					  gint 	  	  dx);

static guint grow_row(guchar   **in_rows,
					  guchar   *mask_row,
					  gint     width,
					  guchar   skel_idx,
					  guint	   pass_number,
					  guchar   color_no_fill,
					  guint    *nb_waiting);

void render_full(gint32             image_ID,
				 GimpDrawable       *drawable,
				 PlugInVals         *vals,
				 PlugInImageVals    *image_vals,
				 PlugInDrawableVals *drawable_vals);
static guint thin_full(gp_bs_full_data *renderer,
					   PlugInVals *vals);
static void grow_full(gp_bs_full_data *renderer,
					  PlugInVals *vals);

static guint thin_tiled(gp_bs_tiled_data *renderer,
						PlugInVals   *vals);
static void grow_tiled(gp_bs_tiled_data *renderer,
					   PlugInVals   *vals);

void render_tiled(gint32             image_ID,
				  GimpDrawable       *drawable,
				  PlugInVals         *vals,
				  PlugInImageVals    *image_vals,
				  PlugInDrawableVals *drawable_vals)
{
	gint         i, nb_bytes_per_pix, nb_bytes;
	gint         x1, y1, x2, y2;
	guint		 nb_used;
	GimpPixelRgn rgn_orig, rgn_inout;
	guchar     	 **in_rows;
	guchar       *empty_row;
	gint         width, height;
	guchar 		 skel_idx = vals->skel_idx;
	gp_bs_tiled_data	renderer;

	printf("render_tiled !!!\n");

	gimp_image_undo_group_start(image_ID);

	renderer.drawable = drawable;

	/* Gets upper left and lower right coordinates */
	gimp_drawable_mask_bounds(drawable->drawable_id,
							  &x1, &y1,
							  &x2, &y2);
	width  = x2 - x1;
	height = y2 - y1;
	renderer.x1 = x1;
	renderer.width = width;
	renderer.y1 = y1;
	renderer.height = height;

	nb_bytes_per_pix = gimp_drawable_bpp(drawable->drawable_id);

	/* Allocate a big enough tile cache */
	gimp_tile_cache_ntiles (3 * (drawable->width /
								 gimp_tile_width () + 1));

	/* Initialises three PixelRgns, one to read original data,
	 * and the other to write output data. That second one will
	 * be merged at the end by the call to
	 * gimp_drawable_merge_shadow() */
	gimp_pixel_rgn_init(&rgn_orig, drawable, x1, y1, width, height,
					    FALSE, FALSE);
	renderer.rgn_orig = &rgn_orig;
	gimp_pixel_rgn_init(&rgn_inout, drawable, x1, y1, width, height,
					    FALSE, TRUE);
	renderer.rgn_inout = &rgn_inout;

	nb_bytes = width * nb_bytes_per_pix;
	in_rows = g_new (guchar *, 3);
	for ( i = 0 ; i < 3; i++)
	{
		in_rows[i] = g_new (guchar, nb_bytes+2);
	}
	renderer.in_rows = in_rows;
	empty_row = g_new0(guchar, nb_bytes+2);
	renderer.empty_row = empty_row;
	in_rows[0][0] = skel_idx;
	in_rows[0][width +1] = skel_idx;
	in_rows[1][0] = skel_idx;
	in_rows[1][width +1] = skel_idx;
	in_rows[2][0] = skel_idx;
	in_rows[2][width +1] = skel_idx;

	nb_used = thin_tiled(&renderer, vals);

	if ( vals->nb_passes > 0 )
	{
		grow_tiled(&renderer, vals);
	}

	gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
	gimp_drawable_update(drawable->drawable_id,
						 x1, y1, width, height);

	for ( i = 0; i < 3 ; i++ )
	{
		g_free (in_rows[i]);
	}
	g_free(renderer.in_rows);
	g_free(renderer.empty_row);

	gimp_image_undo_group_end(image_ID);
}

static guint thin_row(guchar  **in_rows,
					  guchar  *mask_row,
					  gint    width,
					  guchar  skel_idx,
					  gint 	  dx)
{
	guint nb_modified = 0;
	guchar indexed[8];
	gint center_in;
	gint center_out;
	guchar  *out_row = in_rows[1] + 1;
	for ( center_out = 0 ; center_out < width ; center_out += 1 )
	{
		center_in = center_out + 1;
		guchar color = in_rows[1][center_in];
		if ( color == skel_idx && (center_out %2) == dx )
		{
			guint mask = 0;
			guint sh;
			guint which;
			indexed[0] = in_rows[0][center_in -1];
			indexed[1] = in_rows[0][center_in +0];
			indexed[2] = in_rows[0][center_in +1];
			indexed[3] = in_rows[1][center_in +1];
			indexed[4] = in_rows[2][center_in +1];
			indexed[5] = in_rows[2][center_in +0];
			indexed[6] = in_rows[2][center_in -1];
			indexed[7] = in_rows[1][center_in -1];
			for ( sh = 0 ; sh < 8 ; sh++ )
			{
				if ( indexed[sh] == skel_idx )
					mask |= (1 << sh);
			}
			which = sai_thiner[mask];
			if ( which != 0 )
			{
				out_row[center_out] = indexed[which -1];
				if ( mask_row != NULL )
					mask_row[center_out] = 255;
				nb_modified++;
			}
		}
	}
	return nb_modified;
}

static guint grow_row(guchar   **in_rows,
					  guchar   *mask_row,
					  gint     width,
					  guchar   skel_idx,
					  guint	   pass_number,
					  guchar   color_no_fill_idx,
					  guint    *nb_waiting)
{
	guint nb_modified = 0;
	guchar indexed[8];
	gint center_in;
	gint center_out;
	guchar color;
	guchar   *out_row = in_rows[1];
	for ( center_out = 1 ; center_out <= width ; center_out++ )
	{
		center_in = center_out;
		color = in_rows[1][center_in];
		if ( color != skel_idx && color != color_no_fill_idx )
		{
			guint mask = 0;
			gint lacks_boundary = FALSE;
			gint cnx4_w_o = FALSE;
			guint sh;
			guint which;
			indexed[0] = in_rows[0][center_in -1];
			indexed[1] = in_rows[0][center_in +0];
			indexed[2] = in_rows[0][center_in +1];
			indexed[3] = in_rows[1][center_in +1];
			indexed[4] = in_rows[2][center_in +1];
			indexed[5] = in_rows[2][center_in +0];
			indexed[6] = in_rows[2][center_in -1];
			indexed[7] = in_rows[1][center_in -1];
			for ( sh = 0 ; sh < 8 ; sh++ )
			{
				if ( indexed[sh] == skel_idx )
					mask |= (1 << sh);
				else if ( (sh & 1) != 0 )
				{
					if ( indexed[sh] != color && indexed[sh] != color_no_fill_idx )
						lacks_boundary = TRUE;
					else
						cnx4_w_o = TRUE;
				}
			}
			which = sai_grower[mask];
			if ( ( which > 0 ) | lacks_boundary )
			{
				if ( which <= pass_number || (cnx4_w_o & lacks_boundary) )
				{
					out_row[center_out] = skel_idx;
					if ( mask_row != NULL )
						mask_row[center_out -1] = 255;
					nb_modified++;
				}
				else
					(*nb_waiting)++;
			}
		}
	}
	return nb_modified;
}

guint thin_tiled(gp_bs_tiled_data *renderer,
				 PlugInVals   *vals)
{
	GimpPixelRgn *rgn_in = renderer->rgn_orig;
	guchar		 *hold_row, *in_row;
	guint		 i;
	guint		 nb_turns = 0;
	guint		 nb_modified = 0;	// just to keep compiler quiet
	guint		 done = FALSE;
	guint		 x1 = renderer->x1;
	guint		 width = renderer->width;
	guint		 y1 = renderer->y1;
	guint		 height = renderer->height;
	guchar 		 skel_idx = vals->skel_idx;
	gdouble 	 progress_width = 1.0;
	if (vals->nb_passes > 0 )
		progress_width /= 2.0;

	gimp_progress_init ("Shrinking...");

	do
	{
		gint dx = nb_turns % 2;
		gint dy = (nb_turns%4) >> 1;

		if ( (nb_turns % 4) == 0 )
			nb_modified = 0;

		// first line as if skeleton was just over the border
		hold_row = renderer->in_rows[0];
		renderer->in_rows[0] = renderer->empty_row;

		in_row = renderer->in_rows[1];
		gimp_pixel_rgn_get_row(rgn_in, in_row+1, x1, y1, width);

		in_row = renderer->in_rows[2];

		for (i = 0; i < height; i++)
		{
			if ( i < height -1 )
				gimp_pixel_rgn_get_row(rgn_in, in_row+1, x1, y1 +i +1, width);
			else
			{
				hold_row = renderer->in_rows[2];
				renderer->in_rows[2] = renderer->empty_row;
			}

			/* To be done for each tile row */
			if ( (dy + i) % 2 == 0 )
			{
				nb_modified += thin_row(renderer->in_rows,
										NULL, width, skel_idx, dx);
				gimp_pixel_rgn_set_row(renderer->rgn_inout, renderer->in_rows[1] +1,
									   x1, i + y1, width);
			}
			else
			{
				// we have to get these pixels into the out tiles
				if ( nb_turns == 0 )
					gimp_pixel_rgn_set_row(renderer->rgn_inout, renderer->in_rows[1] +1,
										   x1, i + y1, width);
			}

			in_row = ( i == 0 ) ? hold_row : renderer->in_rows[0];
			renderer->in_rows[0] = renderer->in_rows[1];
			renderer->in_rows[1] = ( i == height -1 ) ? hold_row : renderer->in_rows[2];
			renderer->in_rows[2] = in_row;

			if (i % 50 == 0)
			{
				guint p = nb_turns % 4;
				gdouble r = (0.25 * p + (gdouble)i / (4.0 * height));
				guint n = nb_turns / 4;
				gdouble c = 1.0 / (gdouble)(1 << n);
				gdouble w = progress_width * (1.0 -c + 0.5 * c * r);
				gimp_progress_update(w);
			}
		}

		gimp_drawable_flush(renderer->drawable);
		rgn_in = renderer->rgn_inout;
		nb_turns++;
		if ( (nb_turns % 4) == 0 )
			done = ( nb_turns >= 24 || nb_modified == 0 );
	} while ( ! done );

	return nb_turns /4;
}

static void grow_tiled(gp_bs_tiled_data *renderer,
					   PlugInVals   *vals)
{
	GimpPixelRgn *rgn_in = renderer->rgn_inout;
	guchar		 *hold_row, *in_row;
	guint		 i;
	guint		 grow_nb_pass = vals->nb_passes;
	guint		 grow_pass = 1;
	guint		 nb_modified = 0;	// just to keep compiler quiet
	guint		 nb_waiting = 0;
	guint		 done = FALSE;
	guint		 x1 = renderer->x1;
	guint		 width = renderer->width;
	guint		 y1 = renderer->y1;
	guint		 height = renderer->height;
	guchar 		 skel_idx = vals->skel_idx;
	guchar 		 color_no_fill_idx = vals->color_no_fill_idx;
	gdouble		 progress_start = 0.5;
	gdouble		 progress_width = 0.25;

	gimp_progress_set_text("Growing...");

	do
	{
		nb_modified = 0;

		// first line as if skeleton was just over the border
		hold_row = renderer->in_rows[0];
		renderer->in_rows[0] = renderer->empty_row;

		in_row = renderer->in_rows[1];
		gimp_pixel_rgn_get_row(rgn_in, in_row+1, x1, y1, width);

		in_row = renderer->in_rows[2];

		for (i = 0; i < height; i++)
		{
			if ( i < height -1 )
				gimp_pixel_rgn_get_row(rgn_in, in_row+1, x1, y1 +i +1, width);
			else
			{
				hold_row = renderer->in_rows[2];
				renderer->in_rows[2] = renderer->empty_row;
			}

			/* To be done for each tile row */
			nb_modified += grow_row(renderer->in_rows, NULL, width,
									skel_idx, grow_pass, color_no_fill_idx, &nb_waiting);
			gimp_pixel_rgn_set_row(renderer->rgn_inout, renderer->in_rows[1] +1, x1, i + y1, width);

			in_row = ( i == 0 ) ? hold_row : renderer->in_rows[0];
			renderer->in_rows[0] = renderer->in_rows[1];
			renderer->in_rows[1] = ( i == height -1 ) ? hold_row : renderer->in_rows[2];
			renderer->in_rows[2] = in_row;

			if (i % 50 == 0)
			{
				gdouble r = (gdouble)i / (1.0 * height) + grow_pass;
				gdouble w = progress_start + r * progress_width;
				gimp_progress_update(w);
			}
		}

		gimp_drawable_flush(renderer->drawable);

		rgn_in = renderer->rgn_inout;

		if ( grow_pass < grow_nb_pass ) done = nb_waiting == 0;
		else done = TRUE;
		grow_pass++;

	} while ( ! done );
}

static guint thin_full(gp_bs_full_data *renderer,
					   PlugInVals *vals)
{
	guint		 i;
	guint		 nb_turns = 0;
	guint		 nb_modified = 0;	// just to keep compiler quiet
	guint		 done = FALSE;
	guint		 width = renderer->width;
	guint		 height = renderer->height;
	guchar 		 skel_idx = vals->skel_idx;
	gdouble 	 progress_width = 1.0;
	guchar		 *mask_row = NULL;
	if (vals->nb_passes > 0 )
		progress_width /= 2.0;

	gimp_progress_init ("Shrinking...");

	do
	{
		gint dx = nb_turns % 2;
		gint dy = (nb_turns%4) >> 1;

		if ( (nb_turns % 4) == 0 )
			nb_modified = 0;

		// first line as if skeleton was just over the border
		renderer->in_rows[0] = renderer->empty_row;
		renderer->in_rows[1] = renderer->img_bytes;

		for (i = 0; i < height; i++)
		{
			if ( i < height -1 )
				renderer->in_rows[2] = renderer->img_bytes + (width+2) * (i+1);
			else
				renderer->in_rows[2] = renderer->empty_row;
			if ( renderer->mask_bytes != NULL )
				mask_row = renderer->mask_bytes + width * i;

			if ( (dy + i) % 2 == 0 )
				nb_modified += thin_row(renderer->in_rows,
										mask_row, width, skel_idx, dx);

			renderer->in_rows[0] = renderer->in_rows[1];
			renderer->in_rows[1] = renderer->in_rows[2];

			if (i % 50 == 0)
			{
				guint p = nb_turns % 4;
				gdouble r = (0.25 * p + (gdouble)i / (4.0 * height));
				guint n = nb_turns / 4;
				gdouble c = 1.0 / (gdouble)(1 << n);
				gdouble w = progress_width * (1.0 -c + 0.5 * c * r);
				gimp_progress_update(w);
			}
		}

		nb_turns++;
		if ( (nb_turns % 4) == 0 )
			done = ( nb_turns >= 24 || nb_modified == 0 );
	} while ( ! done );

	return nb_turns /4;
}

static void grow_full(gp_bs_full_data *renderer,
					  PlugInVals *vals)
{
	guint		 i;
	guint		 grow_nb_pass = vals->nb_passes;
	guint		 grow_pass = 1;
	guint		 nb_modified = 0;	// just to keep compiler quiet
	guint		 nb_waiting = 0;
	guint		 done = FALSE;
	guint		 width = renderer->width;
	guint		 height = renderer->height;
	guchar 		 skel_idx = vals->skel_idx;
	guchar 		 color_no_fill_idx = vals->color_no_fill_idx;
	gdouble		 progress_start = 0.5;
	gdouble		 progress_width = 0.25;
	guchar		 *mask_row = NULL;

	gimp_progress_set_text("Growing...");

	do
	{
		nb_modified = 0;

		// first line as if skeleton was just over the border
		renderer->in_rows[0] = renderer->empty_row;
		renderer->in_rows[1] = renderer->img_bytes;

		for (i = 0; i < height; i++)
		{
			if ( i < height -1 )
				renderer->in_rows[2] = renderer->img_bytes + (width+2) * (i+1);
			else
				renderer->in_rows[2] = renderer->empty_row;
			if ( renderer->mask_bytes != NULL )
				mask_row = renderer->mask_bytes + width * i;

			nb_modified += grow_row(renderer->in_rows, mask_row, width,
									skel_idx, grow_pass, color_no_fill_idx, &nb_waiting);

			renderer->in_rows[0] = renderer->in_rows[1];
			renderer->in_rows[1] = renderer->in_rows[2];

			if (i % 50 == 0)
			{
				gdouble r = (gdouble)i / (1.0 * height) + grow_pass;
				gdouble w = progress_start + r * progress_width;
				gimp_progress_update(w);
			}
		}

		if ( grow_pass < grow_nb_pass ) done = nb_waiting == 0;
		else done = TRUE;
		grow_pass++;

	} while ( ! done );
}

void render_full(gint32             image_ID,
				 GimpDrawable       *drawable,
				 PlugInVals         *vals,
				 PlugInImageVals    *image_vals,
				 PlugInDrawableVals *drawable_vals)
{
	gint         i;
	gint         x1, y1, x2, y2;
	guint		 nb_used;
	GimpPixelRgn rgn_orig, rgn_out, rgn_mask;
	GimpDrawable *mask_channel;
	guchar     	 **in_rows;
	guchar       *empty_row;
	guchar		 *img_bytes;
	guchar		 *mask_bytes = NULL;
	gint         width, height;
	gint32		 mask_channel_id;
	gp_bs_full_data	renderer;
	gboolean	 should_select_changed = vals->select_changed != 0;

	gimp_image_undo_group_start(image_ID);

	/* Gets upper left and lower right coordinates */
	gimp_drawable_mask_bounds(drawable->drawable_id,
							  &x1, &y1,
							  &x2, &y2);
	width  = x2 - x1;
	height = y2 - y1;
	renderer.x1 = x1;
	renderer.width = width;
	renderer.y1 = y1;
	renderer.height = height;

	// if we use tile cache, we won't be able to separate img bytes from mask bytes :(
	// Bug:100469 in gimp
	gimp_tile_cache_ntiles (0);
	//gimp_tile_cache_ntiles (1 * (drawable->width / gimp_tile_width () + 1));
	gimp_pixel_rgn_init(&rgn_orig, drawable, x1, y1, width, height, FALSE, FALSE);

	in_rows = g_new (guchar *, 3);
	renderer.in_rows = in_rows;

	if ( should_select_changed )
		mask_bytes = g_new0(guchar, width * height);
	renderer.mask_bytes = mask_bytes;

	// we took care of restricting ourselves to 8bit indexed images
	img_bytes = g_new(guchar, (width + 2) * height);
	for ( i = 0 ; i < height ; i++ )
	{
		guint offset = (width+2) * i;
		img_bytes[offset] = 0;
		gimp_pixel_rgn_get_row(&rgn_orig, img_bytes + offset +1, x1, y1 +i, width);
		img_bytes[offset + width +1] = 0;
	}
	renderer.img_bytes = img_bytes;

	empty_row = g_new0(guchar, width+2);
	renderer.empty_row = empty_row;

	nb_used = thin_full(&renderer, vals);
	if ( vals->nb_passes > 0 )
		grow_full(&renderer, vals);

	gimp_pixel_rgn_init(&rgn_out, drawable, x1, y1, width, height, TRUE, TRUE);
	for ( i = 0 ; i < height ; i++ )
	{
		guint offset = (width+2) * i;
		gimp_pixel_rgn_set_row(&rgn_out, img_bytes + offset +1, x1, y1 +i, width);
	}

	gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
	gimp_drawable_update(drawable->drawable_id, x1, y1, width, height);
	gimp_drawable_detach(drawable);

	if ( should_select_changed )
	{
		mask_channel_id = gimp_selection_save(image_ID);
		mask_channel = gimp_drawable_get(mask_channel_id);

		gimp_pixel_rgn_init(&rgn_mask, mask_channel, x1, y1, width, height, TRUE, TRUE);
		for ( i = 0 ; i < height ; i++ )
		{
			guint offset = width * i;
			gimp_pixel_rgn_set_row(&rgn_mask, mask_bytes + offset, x1, y1 +i, width);
		}
		gimp_drawable_merge_shadow(mask_channel_id, TRUE);
		gimp_drawable_update(mask_channel_id, x1, y1, width, height);
		gimp_drawable_detach(mask_channel);

		//gimp_drawable_detach(mask_channel);
		gimp_selection_load(mask_channel_id);

		gimp_image_remove_channel(image_ID, mask_channel_id);
		//gimp_drawable_delete(mask_channel_id);

		g_free(mask_bytes);
	}

	g_free(renderer.in_rows);
	g_free(renderer.empty_row);
	g_free(img_bytes);

	gimp_image_undo_group_end(image_ID);
}

void count_pixels(gint32         image_ID,
				  GimpDrawable      *drawable,
				  gint32			*nb_used,
				  gint32			*nb_pixels)
{
	gint         x1, y1, x2, y2;
	gint         width, height;
	gint         val_idx, row, col;
	GimpPixelRgn rgn;
	GimpPixelRgn *pr1;
	guchar	*row_first, *pix;
	gint32		*color_runner;

	gimp_drawable_mask_bounds(drawable->drawable_id,
							  &x1, &y1,
							  &x2, &y2);
	width  = x2 - x1;
	height = y2 - y1;

	for ( val_idx = 0 ; val_idx < 64 ; val_idx++ ) nb_pixels[val_idx] = 0;

	gimp_pixel_rgn_init(&rgn, drawable, x1, y1, width, height, FALSE, FALSE);
	for ( pr1 = (GimpPixelRgn *)gimp_pixel_rgns_register(1, &rgn) ;
		  pr1 != NULL ;
		  pr1 = gimp_pixel_rgns_process (pr1))
	{
		row_first = rgn.data;
		for ( row = 0 ; row < rgn.h ; row++ )
		{
			pix = row_first;
			for ( col = 0 ; col < rgn.w ; col++ )
			{
				guchar c = *pix;
				guint i = c;
				pix++;
				nb_pixels[i] += 1;
			}
			row_first += rgn.rowstride;
		}
	}

	*nb_used = 0;
	color_runner = nb_pixels;
	for ( val_idx = 0 ; val_idx < 256 ; val_idx++ )
	{
		if ( *color_runner != 0 )
		{
			(*nb_used)++;
		}
		color_runner++;
	}
}


