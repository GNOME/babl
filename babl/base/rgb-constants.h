/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

/*
 * These sRGB Y values have been adapted to the ICC spec D50 illuminant.
 * They match the Y values in the GEGL and GIMP built-in sRGB profiles,
 * which match the Y values in the ArgyllCMS sRGB.icm profile.
 *
 * For more information, see this thread and these bug reports:
 * https://mail.gnome.org/archives/gimp-developer-list/2013-September/msg00113.html
 * https://bugzilla.gnome.org/show_bug.cgi?id=723787
 * https://bugzilla.gnome.org/show_bug.cgi?id=724822
 */

#define RGB_LUMINANCE_RED    (0.22248840)
#define RGB_LUMINANCE_GREEN  (0.71690369)
#define RGB_LUMINANCE_BLUE   (0.06060791)
