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

#ifndef __RENDER_H__
#define __RENDER_H__


/*  Public functions  */

void render_tiled(gint32         image_ID,
				  GimpDrawable       *drawable,
				  PlugInVals         *vals,
				  PlugInImageVals    *image_vals,
				  PlugInDrawableVals *drawable_vals);

void render_full(gint32         image_ID,
				 GimpDrawable       *drawable,
				 PlugInVals         *vals,
				 PlugInImageVals    *image_vals,
				 PlugInDrawableVals *drawable_vals);

void count_pixels(gint32         image_ID,
				  GimpDrawable       *drawable,
				  gint32			*nb_used,
				  gint32			*nb_pixels);

#endif /* __RENDER_H__ */
