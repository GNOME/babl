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
 * These primaries have been adapted for a D50 illuminant.
 *
 * They were taken from here:
 * http://www.brucelindbloom.com/WorkingSpaceInfo.html#AdaptedPrimaries
 *
 * For more information, see this thread:
 * https://mail.gnome.org/archives/gimp-developer-list/2013-September/msg00113.html
 */

#define RGB_LUMINANCE_RED    (0.222491)
#define RGB_LUMINANCE_GREEN  (0.716888)
#define RGB_LUMINANCE_BLUE   (0.060621)
