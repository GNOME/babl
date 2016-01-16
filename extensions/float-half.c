/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2015 Daniel Sabo
 *               2016 Øyvind Kolås
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

/* Copyright:   (c) 2009 by James Tursa, All Rights Reserved
 *
 *  This code uses the BSD License:
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are 
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the distribution
 *      
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * halfprecision converts the input argument to/from a half precision floating
 * point bit pattern corresponding to IEEE 754r. The bit pattern is stored in a
 * uint16 class variable. Please note that halfprecision is *not* a class. That
 * is, you cannot do any arithmetic with the half precision bit patterns.
 * halfprecision is simply a function that converts the IEEE 754r half precision
 * bit pattern to/from other numeric MATLAB variables. You can, however, take
 * the half precision bit patterns, convert them to single or double, do the
 * operation, and then convert the result back manually.
 *
 * 1 bit sign bit
 * 5 bits exponent, biased by 15
 * 10 bits mantissa, hidden leading bit, normalized to 1.0
 *
 * Special floating point bit patterns recognized and supported:
 *
 * All exponent bits zero:
 * - If all mantissa bits are zero, then number is zero (possibly signed)
 * - Otherwise, number is a denormalized bit pattern
 *
 * All exponent bits set to 1:
 * - If all mantissa bits are zero, then number is +Infinity or -Infinity
 * - Otherwise, number is NaN (Not a Number)
 */

#include "config.h"

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "extensions/util.h"

static void halfp2singles(void *target, const void *source, long numel)
{
    uint16_t *hp = (uint16_t *) source; // Type pun input as an unsigned 16-bit int
    uint32_t *xp = (uint32_t *) target; // Type pun output as an unsigned 32-bit int
    uint16_t h, hs, he, hm;
    uint32_t xs, xe, xm;
    int32_t xes;
    int e;
    
    if( source == NULL || target == NULL ) // Nothing to convert (e.g., imag part of pure real)
        return;
    while( numel-- ) {
        h = *hp++;
        if( (h & 0x7FFFu) == 0 ) {  // Signed zero
            *xp++ = ((uint32_t) h) << 16;  // Return the signed zero
        } else { // Not zero
            hs = h & 0x8000u;  // Pick off sign bit
            he = h & 0x7C00u;  // Pick off exponent bits
            hm = h & 0x03FFu;  // Pick off mantissa bits
            if( he == 0 ) {  // Denormal will convert to normalized
                e = -1; // The following loop figures out how much extra to adjust the exponent
                do {
                    e++;
                    hm <<= 1;
                } while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
                xs = ((uint32_t) hs) << 16; // Sign bit
                xes = ((int32_t) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
                xe = (uint32_t) (xes << 23); // Exponent
                xm = ((uint32_t) (hm & 0x03FFu)) << 13; // Mantissa
                *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
            } else if( he == 0x7C00u ) {  // Inf or NaN (all the exponent bits are set)
                if( hm == 0 ) { // If mantissa is zero ...
                    *xp++ = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); // Signed Inf
                } else {
                    *xp++ = (uint32_t) 0xFFC00000u; // NaN, only 1st mantissa bit set
                }
            } else { // Normalized number
                xs = ((uint32_t) hs) << 16; // Sign bit
                xes = ((int32_t) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
                xe = (uint32_t) (xes << 23); // Exponent
                xm = ((uint32_t) hm) << 13; // Mantissa
                *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
            }
        }
    }
}

static void singles2halfp(void *target, const void *source, long numel)
{
    uint16_t *hp = (uint16_t *) target; // Type pun output as an unsigned 16-bit int
    uint32_t *xp = (uint32_t *) source; // Type pun input as an unsigned 32-bit int
    uint16_t    hs, he, hm;
    uint32_t x, xs, xe, xm;
    int hes;
    
    if( source == NULL || target == NULL ) { // Nothing to convert (e.g., imag part of pure real)
        return;
    }
    while( numel-- ) {
        x = *xp++;
        if( (x & 0x7FFFFFFFu) == 0 ) {  // Signed zero
            *hp++ = (uint16_t) (x >> 16);  // Return the signed zero
        } else { // Not zero
            xs = x & 0x80000000u;  // Pick off sign bit
            xe = x & 0x7F800000u;  // Pick off exponent bits
            xm = x & 0x007FFFFFu;  // Pick off mantissa bits
            if( xe == 0 ) {  // Denormal will underflow, return a signed zero
                *hp++ = (uint16_t) (xs >> 16);
            } else if( xe == 0x7F800000u ) {  // Inf or NaN (all the exponent bits are set)
                if( xm == 0 ) { // If mantissa is zero ...
                    *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
                } else {
                    *hp++ = (uint16_t) 0xFE00u; // NaN, only 1st mantissa bit set
                }
            } else { // Normalized number
                hs = (uint16_t) (xs >> 16); // Sign bit
                hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
                if( hes >= 0x1F ) {  // Overflow
                    *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
                } else if( hes <= 0 ) {  // Underflow
                    if( (14 - hes) > 24 ) {  // Mantissa shifted all the way off & no rounding possibility
                        hm = (uint16_t) 0u;  // Set mantissa to zero
                    } else {
                        xm |= 0x00800000u;  // Add the hidden leading bit
                        hm = (uint16_t) (xm >> (14 - hes)); // Mantissa
                        if( (xm >> (13 - hes)) & 0x00000001u ) // Check for rounding
                            hm += (uint16_t) 1u; // Round, might overflow into exp bit, but this is OK
                    }
                    *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
                } else {
                   he = (uint16_t) (hes << 10); // Exponent
                    hm = (uint16_t) (xm >> 13); // Mantissa
                    if( xm & 0x00001000u ) // Check for rounding
                        *hp++ = (hs | he | hm) + (uint16_t) 1u; // Round, might overflow to inf, this is OK
                    else
                        *hp++ = (hs | he | hm);  // No rounding
                }
            }
        }
    }
}

static inline long
conv_yHalf_yF (const uint16_t *src, float *dst, long samples)
{
  halfp2singles(dst, src, samples);
  return samples;
}

static long
conv_yaHalf_yaF (const uint16_t *src, float *dst, long samples)
{
  return conv_yHalf_yF (src, dst, samples * 2) / 2;
}

static long
conv_rgbHalf_rgbF (const uint16_t *src, float *dst, long samples)
{
  return conv_yHalf_yF (src, dst, samples * 3) / 3;
}

static long
conv_rgbaHalf_rgbaF (const uint16_t *src, float *dst, long samples)
{
  return conv_yHalf_yF (src, dst, samples * 4) / 4;
}

static inline long
conv_yF_yHalf (const float *src, uint16_t *dst, long samples)
{
  singles2halfp (dst, src, samples);
  return samples;
}

static long
conv_yaF_yaHalf (const float *src, uint16_t *dst, long samples)
{
  return conv_yF_yHalf (src, dst, samples * 2) / 2;
}

static long
conv_rgbF_rgbHalf (const float *src, uint16_t *dst, long samples)
{
  return conv_yF_yHalf (src, dst, samples * 3) / 3;
}

static long
conv_rgbaF_rgbaHalf (const float *src, uint16_t *dst, long samples)
{
  return conv_yF_yHalf (src, dst, samples * 4) / 4;
}

int init (void);

int
init (void)
{
  const Babl *rgbaF_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaHalf_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("half"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaF_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaHalf_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("half"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgbHalf_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("half"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgbF_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgbHalf_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("half"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yaHalf_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("half"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yaF_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yaHalf_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("half"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *yHalf_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("half"),
    babl_component ("Y"),
    NULL);
  const Babl *yF_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
    NULL);
  const Babl *yHalf_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("half"),
    babl_component ("Y'"),
    NULL);

#define CONV(src, dst) \
{ \
  babl_conversion_new (src ## _linear, dst ## _linear, "linear", conv_ ## src ## _ ## dst, NULL); \
  babl_conversion_new (src ## _gamma, dst ## _gamma, "linear", conv_ ## src ## _ ## dst, NULL); \
}

  CONV(rgbaHalf, rgbaF);
  CONV(rgbHalf,  rgbF);
  CONV(yaHalf,   yaF);
  CONV(yHalf,    yF);
  CONV(rgbaF,    rgbaHalf);
  CONV(rgbF,     rgbHalf);
  CONV(yaF,      yaHalf);
  CONV(yF,       yHalf);

  return 0;
}
