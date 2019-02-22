/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Øyvind Kolås.
 *
 * MATLAB (R) is a trademark of The Mathworks (R) Corporation
 *
 * Function:    halfprecision
 * Filename:    halfprecision.c
 * Programmer:  James Tursa
 * Version:     1.0
 * Date:        March 3, 2009
 * Copyright:   (c) 2009 by James Tursa, All Rights Reserved
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
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "babl.h"
#include "babl-classes.h"
#include "babl-ids.h"
#include "babl-base.h"

static int next = 1; /* should be 0 for big endian */

//-----------------------------------------------------------------------------

static void 
doubles2halfp(void *target, 
              void *source, 
              long  numel)
{
    uint16_t *hp = (uint16_t *) target; // Type pun output as an unsigned 16-bit int
    uint32_t *xp = (uint32_t *) source; // Type pun input as an unsigned 32-bit int
    uint16_t    hs, he, hm;
    uint32_t x, xs, xe, xm;
    int hes;

    xp += next;  // Little Endian adjustment if necessary
    
    if( source == NULL || target == NULL ) { // Nothing to convert (e.g., imag part of pure real)
        return;
    }
    while( numel-- ) {
        x = *xp++; xp++; // The extra xp++ is to skip over the remaining 32 bits of the mantissa
        if( (x & 0x7FFFFFFFu) == 0 ) {  // Signed zero
            *hp++ = (uint16_t) (x >> 16);  // Return the signed zero
        } else { // Not zero
            xs = x & 0x80000000u;  // Pick off sign bit
            xe = x & 0x7FF00000u;  // Pick off exponent bits
            xm = x & 0x000FFFFFu;  // Pick off mantissa bits
            if( xe == 0 ) {  // Denormal will underflow, return a signed zero
                *hp++ = (uint16_t) (xs >> 16);
            } else if( xe == 0x7FF00000u ) {  // Inf or NaN (all the exponent bits are set)
                if( xm == 0 ) { // If mantissa is zero ...
                    *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
                } else {
                    *hp++ = (uint16_t) 0xFE00u; // NaN, only 1st mantissa bit set
                }
            } else { // Normalized number
                hs = (uint16_t) (xs >> 16); // Sign bit
                hes = ((int)(xe >> 20)) - 1023 + 15; // Exponent unbias the double, then bias the halfp
                if( hes >= 0x1F ) {  // Overflow
                    *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
                } else if( hes <= 0 ) {  // Underflow
                    if( (10 - hes) > 21 ) {  // Mantissa shifted all the way off & no rounding possibility
                        hm = (uint16_t) 0u;  // Set mantissa to zero
                    } else {
                        xm |= 0x00100000u;  // Add the hidden leading bit
                        hm = (uint16_t) (xm >> (11 - hes)); // Mantissa
                        if( (xm >> (10 - hes)) & 0x00000001u ) // Check for rounding
                            hm += (uint16_t) 1u; // Round, might overflow into exp bit, but this is OK
                    }
                    *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
                } else {
                    he = (uint16_t) (hes << 10); // Exponent
                    hm = (uint16_t) (xm >> 10); // Mantissa
                    if( xm & 0x00000200u ) // Check for rounding
                        *hp++ = (hs | he | hm) + (uint16_t) 1u; // Round, might overflow to inf, this is OK
                    else
                        *hp++ = (hs | he | hm);  // No rounding
                }
            }
        }
    }
}

static void 
halfp2doubles(void *target, 
              void *source, 
              long  numel)
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
        uint32_t x;

        h = *hp++;
        if( (h & 0x7FFFu) == 0 ) {  // Signed zero
            x = ((uint32_t) h) << 16;  // Return the signed zero
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
                xes = ((int32_t) (he >> 10)) - 15 + 1023 - e; // Exponent unbias the halfp, then bias the double
                xe = (uint32_t) (xes << 20); // Exponent
                xm = ((uint32_t) (hm & 0x03FFu)) << 10; // Mantissa
                x = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
            } else if( he == 0x7C00u ) {  // Inf or NaN (all the exponent bits are set)
                if( hm == 0 ) { // If mantissa is zero ...
                    x = (((uint32_t) hs) << 16) | ((uint32_t) 0x7FF00000u); // Signed Inf
                } else {
                    x = (uint32_t) 0xFFF80000u; // NaN, only the 1st mantissa bit set
                }
            } else {
                xs = ((uint32_t) hs) << 16; // Sign bit
                xes = ((int32_t) (he >> 10)) - 15 + 1023; // Exponent unbias the halfp, then bias the double
                xe = (uint32_t) (xes << 20); // Exponent
                xm = ((uint32_t) hm) << 10; // Mantissa
                x = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
            }
        }

       xp[1 - next] = 0;
       xp[next] = x;

       xp += 2;
    }
}


static void
singles2halfp(uint16_t       *hp, 
              const uint32_t *xp,
              int             numel)
{
    
    uint16_t    hs, he, hm;
    uint32_t x, xs, xe, xm;
    int hes;    
    
   
    
    if(  hp== NULL ||  xp== NULL ) { // Nothing to convert (e.g., imag part of pure real)
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
    return;
}


    
   
                       
//-----------------------------------------------------------------------------
//
// Routine:  halfp2singles
//
// Input:  source = address of 16-bit data to convert
//         numel  = Number of values at that address to convert
//
// Output: target = Address of 32-bit floating point data to hold output (numel values)
//         
//
// Programmer:  James Tursa
//
//-----------------------------------------------------------------------------

static void
halfp2singles(uint32_t       *xp, 
              const uint16_t *hp, 
              int             numel)
{
    
    uint16_t h, hs, he, hm;
    uint32_t xs, xe, xm;
    int32_t xes;
    int e;


    
    if( xp == NULL || hp == NULL ) // Nothing to convert (e.g., imag part of pure real)
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
    return;
}





static void
convert_double_half (BablConversion *conversion,
                     char           *src,
                     char           *dst,
                     int             src_pitch,
                     int             dst_pitch,
                     long            n)
{
  while (n--)
    {
      doubles2halfp (dst, src, 1);
      dst             += dst_pitch;
      src             += src_pitch;
    }
}

static void
convert_half_double (BablConversion *conversion,
                     char           *src,
                     char           *dst,
                     int             src_pitch,
                     int             dst_pitch,
                     long            n)
{
  while (n--)
    {
      halfp2doubles (dst, src, 1);
      dst              += dst_pitch;
      src              += src_pitch;
    }
}


static void
convert_float_half (BablConversion *conversion,
                    char           *src,
                    char           *dst,
                    int             src_pitch,
                    int             dst_pitch,
                    long            n)
{
  while (n--)
    {
      singles2halfp ((void*)dst, (void*)src, 1);
      dst             += dst_pitch;
      src             += src_pitch;
    }
}

static void
convert_half_float (BablConversion *conversion,
                    char           *src,
                    char           *dst,
                    int             src_pitch,
                    int             dst_pitch,
                    long            n)
{
  while (n--)
    {
      halfp2singles ((void*)dst, (void*)src, 1);
      dst              += dst_pitch;
      src              += src_pitch;
    }
}




void
babl_base_type_half (void)
{
  babl_type_new (
    "half",
    "id", BABL_HALF,
    "bits", 16,
    NULL);

  babl_conversion_new (
    babl_type_from_id (BABL_HALF),
    babl_type_from_id (BABL_DOUBLE),
    "plane", convert_half_double,
    NULL
  );

  babl_conversion_new (
    babl_type_from_id (BABL_DOUBLE),
    babl_type_from_id (BABL_HALF),
    "plane", convert_double_half,
    NULL
  );

  babl_conversion_new (
    babl_type_from_id (BABL_HALF),
    babl_type_from_id (BABL_FLOAT),
    "plane", convert_half_float,
    NULL
  );

  babl_conversion_new (
    babl_type_from_id (BABL_FLOAT),
    babl_type_from_id (BABL_HALF),
    "plane", convert_float_half,
    NULL
  );
}
