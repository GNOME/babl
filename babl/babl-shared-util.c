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
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stddef.h>
#include <stdint.h>

#include "babl-shared-util.h"


void
_babl_float_to_half (void        *halfp,
                     const float *floatp,
                     int          numel)
{
  uint16_t       *hp = (uint16_t *) halfp;
  const uint32_t *xp = (const uint32_t *) floatp;
  uint16_t        hs, he, hm;
  uint32_t        x, xs, xe, xm;
  int             hes;

  if (hp == NULL || xp == NULL)
    {
      /* Nothing to convert (e.g., imag part of pure real) */
      return;
    }

  while (numel--)
    {
      x = *xp++;
      if ((x & 0x7FFFFFFFu) == 0)
        {
          /* Return the signed zero */
          *hp++ = (uint16_t) (x >> 16);
        }
      else
        {
          /* Not zero */
          xs = x & 0x80000000u;  /* Pick off sign bit      */
          xe = x & 0x7F800000u;  /* Pick off exponent bits */
          xm = x & 0x007FFFFFu;  /* Pick off mantissa bits */
          if (xe == 0)
            {
              /* Denormal will underflow, return a signed zero */
              *hp++ = (uint16_t) (xs >> 16);
            }
          else if (xe == 0x7F800000u)
            {
              /* Inf or NaN (all the exponent bits are set) */
              if (xm == 0)
                {
                  /* If mantissa is zero ... */
                  *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); /* Signed Inf */
                }
              else
                {
                  *hp++ = (uint16_t) 0xFE00u; /* NaN, only 1st mantissa bit set */
                }
            }
          else
            {
              /* Normalized number */
              hs = (uint16_t) (xs >> 16); /* Sign bit */
              hes = ((int)(xe >> 23)) - 127 + 15; /* Exponent unbias the single, then bias the halfp */
              if (hes >= 0x1F)
                {
                  /* Overflow */
                  *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); /* Signed Inf */
                }
              else if (hes <= 0)
                {
                  /* Underflow */
                  if ((14 - hes) > 24)
                    {
                      /* Mantissa shifted all the way off & no rounding possibility */
                      hm = (uint16_t) 0u;  /* Set mantissa to zero */
                    }
                  else
                    {
                      xm |= 0x00800000u;  /* Add the hidden leading bit */
                      hm = (uint16_t) (xm >> (14 - hes)); /* Mantissa */
                      if ((xm >> (13 - hes)) & 0x00000001u) /* Check for rounding */
                        hm += (uint16_t) 1u; /* Round, might overflow into exp bit, but this is OK */
                    }
                  *hp++ = (hs | hm); /* Combine sign bit and mantissa bits, biased exponent is zero */
                }
              else
                {
                  he = (uint16_t) (hes << 10); /* Exponent */
                  hm = (uint16_t) (xm >> 13);  /* Mantissa */
                  if (xm & 0x00001000u) /* Check for rounding */
                    *hp++ = (hs | he | hm) + (uint16_t) 1u; /* Round, might overflow to inf, this is OK */
                  else
                    *hp++ = (hs | he | hm);  /* No rounding */
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------\
 *
 * Routine:  _babl_half_to_float (formerly halfp2singles)
 *
 * Input:  source = address of 16-bit data to convert
 *         numel  = Number of values at that address to convert
 *
 * Output: target = Address of 32-bit floating point data to hold output (numel values)
 *
 *
 * Programmer:  James Tursa
 *
\*-----------------------------------------------------------------------------*/
void
_babl_half_to_float (float      *floatp,
                     const void *halfp,
                     int         numel)
{
  uint32_t       *xp = (uint32_t *) floatp;
  const uint16_t *hp = (const uint16_t *) halfp;
  uint16_t        h, hs, he, hm;
  uint32_t        xs, xe, xm;
  int32_t         xes;
  int             e;

  if (xp == NULL || hp == NULL)
    /* Nothing to convert (e.g., imag part of pure real) */
    return;

  while (numel--)
    {
      h = *hp++;
      if ((h & 0x7FFFu) == 0)
        {
          /* Return the signed zero */
          *xp++ = ((uint32_t) h) << 16;
        }
      else
        {
          /* Not zero */
          hs = h & 0x8000u; /* Pick off sign bit      */
          he = h & 0x7C00u; /* Pick off exponent bits */
          hm = h & 0x03FFu; /* Pick off mantissa bits */
          if (he == 0)
            {
              /* Denormal will convert to normalized */
              e = -1; /* The following loop figures out how much extra to adjust the exponent */
              do
                {
                  e++;
                  hm <<= 1;
                }
              while ((hm & 0x0400u) == 0); /* Shift until leading bit overflows into exponent bit */
              xs = ((uint32_t) hs) << 16; /* Sign bit */
              xes = ((int32_t) (he >> 10)) - 15 + 127 - e; /* Exponent unbias the halfp, then bias the single */
              xe = (uint32_t) (xes << 23); /* Exponent */
              xm = ((uint32_t) (hm & 0x03FFu)) << 13; /* Mantissa */
              *xp++ = (xs | xe | xm); /* Combine sign bit, exponent bits, and mantissa bits */
            }
          else if (he == 0x7C00u)
            {
              /* Inf or NaN (all the exponent bits are set) */
              if (hm == 0)
                {
                  /* If mantissa is zero ... */
                  *xp++ = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); /* Signed Inf */
                }
              else
                {
                  *xp++ = (uint32_t) 0xFFC00000u; /* NaN, only 1st mantissa bit set */
                }
            }
          else
            {
              /* Normalized number */
              xs = ((uint32_t) hs) << 16; /* Sign bit */
              xes = ((int32_t) (he >> 10)) - 15 + 127; /* Exponent unbias the halfp, then bias the single */
              xe = (uint32_t) (xes << 23); /* Exponent */
              xm = ((uint32_t) hm) << 13; /* Mantissa */
              *xp++ = (xs | xe | xm); /* Combine sign bit, exponent bits, and mantissa bits */
            }
        }
    }
}
