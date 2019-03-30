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

#ifndef __MAIN_H__
#define __MAIN_H__


typedef struct
{
  GimpRGB	skel_color;
  guchar	skel_idx;
  guchar	nb_passes;
  GimpRGB	color_no_fill;
  guchar	color_no_fill_idx;
  gboolean	select_changed;
} PlugInVals;

typedef struct
{
  gint32    image_id;
} PlugInImageVals;

typedef struct
{
  gint32    drawable_id;
} PlugInDrawableVals;


/*  Default values  */

extern const PlugInVals         default_vals;
extern const PlugInImageVals    default_image_vals;
extern const PlugInDrawableVals default_drawable_vals;


#endif /* __MAIN_H__ */
