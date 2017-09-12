/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017 Øyvind Kolås.
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

#define MAX_TRCS   100

/* FIXME: choose parameters more intelligently */
#define POLY_GAMMA_X0     (  0.5 / 255.0)
#define POLY_GAMMA_X1     (254.5 / 255.0)
#define POLY_GAMMA_DEGREE 6
#define POLY_GAMMA_SCALE  2

#include "config.h"
#include "babl-internal.h"
#include "base/util.h"

static BablTRC trc_db[MAX_TRCS];

static inline float _babl_trc_linear (const Babl *trc_, float value)
{
  return value;
}

static inline float babl_trc_lut_from_linear (const Babl *trc_, float x)
{
  BablTRC *trc = (void*)trc_;
  int entry;
  float ret, diff;

  entry = x * (trc->lut_size-1);
  diff =  ( (x * (trc->lut_size-1)) - entry);

  if (entry >= trc->lut_size -1)
  {
    entry = trc->lut_size - 1;
    diff = 0.0;
  }
  else if (entry < 0) entry = 0;

  if (diff > 0.0)
  {
    ret = trc->inv_lut[entry] * (1.0 - diff) + trc->inv_lut[entry+1] * diff;
  }
  else
  {
    ret = trc->inv_lut[entry];
  }
  return ret;
}

static inline float babl_trc_lut_to_linear (const Babl *trc_, float x)
{
  BablTRC *trc = (void*)trc_;
  int entry;
  float ret, diff;

  entry = x * (trc->lut_size-1);
  diff =  ( (x * (trc->lut_size-1)) - entry);

  if (entry >= trc->lut_size) entry = trc->lut_size - 1;
  else if (entry < 0) entry = 0;

  if (diff > 0.0 && entry < trc->lut_size - 1)
  {
    ret = trc->lut[entry] * (1.0 - diff) + trc->lut[entry+1] * diff;
  }
  else
  {
    ret = trc->lut[entry];
  }
  return ret;
}

/* origin: FreeBSD /usr/src/lib/msun/src/e_powf.c, copied from musl */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

//#include "libm.h"

/* Get a 32 bit int from a float.  */
#define GET_FLOAT_WORD(w,d)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.f = (d);                                    \
  (w) = __u.i;                                    \
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,w)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.i = (w);                                    \
  (d) = __u.f;                                    \
} while (0)


static const float
bp[]   = {1.0, 1.5,},
dp_h[] = { 0.0, 5.84960938e-01,}, /* 0x3f15c000 */
dp_l[] = { 0.0, 1.56322085e-06,}, /* 0x35d1cfdc */
two24  =  16777216.0,  /* 0x4b800000 */
huge   =  1.0e30,
tiny   =  1.0e-30,
/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
L1 =  6.0000002384e-01, /* 0x3f19999a */
L2 =  4.2857143283e-01, /* 0x3edb6db7 */
L3 =  3.3333334327e-01, /* 0x3eaaaaab */
L4 =  2.7272811532e-01, /* 0x3e8ba305 */
L5 =  2.3066075146e-01, /* 0x3e6c3255 */
L6 =  2.0697501302e-01, /* 0x3e53f142 */
P1 =  1.6666667163e-01, /* 0x3e2aaaab */
P2 = -2.7777778450e-03, /* 0xbb360b61 */
P3 =  6.6137559770e-05, /* 0x388ab355 */
P4 = -1.6533901999e-06, /* 0xb5ddea0e */
P5 =  4.1381369442e-08, /* 0x3331bb4c */
lg2     =  6.9314718246e-01, /* 0x3f317218 */
lg2_h   =  6.93145752e-01,   /* 0x3f317200 */
lg2_l   =  1.42860654e-06,   /* 0x35bfbe8c */
ovt     =  4.2995665694e-08, /* -(128-log2(ovfl+.5ulp)) */
cp      =  9.6179670095e-01, /* 0x3f76384f =2/(3ln2) */
cp_h    =  9.6191406250e-01, /* 0x3f764000 =12b cp */
cp_l    = -1.1736857402e-04; /* 0xb8f623c6 =tail of cp_h */
#if 0
ivln2   =  1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
ivln2_h =  1.4426879883e+00, /* 0x3fb8aa00 =16b 1/ln2*/
ivln2_l =  7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/
#endif

static inline float obabl_powf(float x, float y)
{
	float z,ax,z_h,z_l,p_h,p_l;
	float y1,t1,t2,r,s,sn,t,u,v,w;
	int32_t i,j,k,yisint,n;
	int32_t hx,hy,ix,iy,is;

	GET_FLOAT_WORD(hx, x);
	GET_FLOAT_WORD(hy, y);
	ix = hx & 0x7fffffff;
	iy = hy & 0x7fffffff;

#if 0
	/* x**0 = 1, even if x is NaN */
	if (iy == 0)
		return 1.0f;
#endif
	/* 1**y = 1, even if y is NaN */
	if (hx == 0x3f800000)
		return 1.0f;
	/* NaN if either arg is NaN */
	if (ix > 0x7f800000 || iy > 0x7f800000)
		return x + y;

	/* determine if y is an odd int when x < 0
	 * yisint = 0       ... y is not an integer
	 * yisint = 1       ... y is an odd int
	 * yisint = 2       ... y is an even int
	 */
	yisint  = 0;
	if (hx < 0) {
		if (iy >= 0x4b800000)
			yisint = 2; /* even integer y */
		else if (iy >= 0x3f800000) {
			k = (iy>>23) - 0x7f;         /* exponent */
			j = iy>>(23-k);
			if ((j<<(23-k)) == iy)
				yisint = 2 - (j & 1);
		}
	}
#if 0
	/* special value of y */
	if (iy == 0x7f800000) {  /* y is +-inf */
		if (ix == 0x3f800000)      /* (-1)**+-inf is 1 */
			return 1.0f;
		else if (ix > 0x3f800000)  /* (|x|>1)**+-inf = inf,0 */
			return hy >= 0 ? y : 0.0f;
		else                       /* (|x|<1)**+-inf = 0,inf */
			return hy >= 0 ? 0.0f: -y;
	}
	if (iy == 0x3f800000)    /* y is +-1 */
		return hy >= 0 ? x : 1.0f/x;
	if (hy == 0x40000000)    /* y is 2 */
		return x*x;
	if (hy == 0x3f000000) {  /* y is  0.5 */
		if (hx >= 0)     /* x >= +0 */
			return sqrtf(x);
	}
#endif

	ax = fabsf(x);
	/* special value of x */
	if (ix == 0x7f800000 || ix == 0 || ix == 0x3f800000) { /* x is +-0,+-inf,+-1 */
		z = ax;
		if (hy < 0)  /* z = (1/|x|) */
			z = 1.0f/z;
		if (hx < 0) {
			if (((ix-0x3f800000)|yisint) == 0) {
				z = (z-z)/(z-z); /* (-1)**non-int is NaN */
			} else if (yisint == 1)
				z = -z;          /* (x<0)**odd = -(|x|**odd) */
		}
		return z;
	}

	sn = 1.0f; /* sign of result */
	if (hx < 0) {
		if (yisint == 0) /* (x<0)**(non-int) is NaN */
			return (x-x)/(x-x);
		if (yisint == 1) /* (x<0)**(odd int) */
			sn = -1.0f;
	}

#if 0
	/* |y| is huge */
	if (iy > 0x4d000000) { /* if |y| > 2**27 */
		/* over/underflow if x is not close to one */
		if (ix < 0x3f7ffff8)
			return hy < 0 ? sn*huge*huge : sn*tiny*tiny;
		if (ix > 0x3f800007)
			return hy > 0 ? sn*huge*huge : sn*tiny*tiny;
		/* now |1-x| is tiny <= 2**-20, suffice to compute
		   log(x) by x-x^2/2+x^3/3-x^4/4 */
		t = ax - 1;     /* t has 20 trailing zeros */
		w = (t*t)*(0.5f - t*(0.333333333333f - t*0.25f));
		u = ivln2_h*t;  /* ivln2_h has 16 sig. bits */
		v = t*ivln2_l - w*ivln2;
		t1 = u + v;
		GET_FLOAT_WORD(is, t1);
		SET_FLOAT_WORD(t1, is & 0xfffff000);
		t2 = v - (t1-u);
	} else 
#endif
{
		float s2,s_h,s_l,t_h,t_l;
		n = 0;
		/* take care subnormal number */
		if (ix < 0x00800000) {
			ax *= two24;
			n -= 24;
			GET_FLOAT_WORD(ix, ax);
		}
		n += ((ix)>>23) - 0x7f;
		j = ix & 0x007fffff;
		/* determine interval */
		ix = j | 0x3f800000;     /* normalize ix */
		if (j <= 0x1cc471)       /* |x|<sqrt(3/2) */
			k = 0;
		else if (j < 0x5db3d7)   /* |x|<sqrt(3)   */
			k = 1;
		else {
			k = 0;
			n += 1;
			ix -= 0x00800000;
		}
		SET_FLOAT_WORD(ax, ix);

		/* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
		u = ax - bp[k];   /* bp[0]=1.0, bp[1]=1.5 */
		v = 1.0f/(ax+bp[k]);
		s = u*v;
		s_h = s;
		GET_FLOAT_WORD(is, s_h);
		SET_FLOAT_WORD(s_h, is & 0xfffff000);
		/* t_h=ax+bp[k] High */
		is = ((ix>>1) & 0xfffff000) | 0x20000000;
		SET_FLOAT_WORD(t_h, is + 0x00400000 + (k<<21));
		t_l = ax - (t_h - bp[k]);
		s_l = v*((u - s_h*t_h) - s_h*t_l);
		/* compute log(ax) */
		s2 = s*s;
		r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
		r += s_l*(s_h+s);
		s2 = s_h*s_h;
		t_h = 3.0f + s2 + r;
		GET_FLOAT_WORD(is, t_h);
		SET_FLOAT_WORD(t_h, is & 0xfffff000);
		t_l = r - ((t_h - 3.0f) - s2);
		/* u+v = s*(1+...) */
		u = s_h*t_h;
		v = s_l*t_h + t_l*s;
		/* 2/(3log2)*(s+...) */
		p_h = u + v;
		GET_FLOAT_WORD(is, p_h);
		SET_FLOAT_WORD(p_h, is & 0xfffff000);
		p_l = v - (p_h - u);
		z_h = cp_h*p_h;  /* cp_h+cp_l = 2/(3*log2) */
		z_l = cp_l*p_h + p_l*cp+dp_l[k];
		/* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
		t = (float)n;
		t1 = (((z_h + z_l) + dp_h[k]) + t);
		GET_FLOAT_WORD(is, t1);
		SET_FLOAT_WORD(t1, is & 0xfffff000);
		t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
	}

	/* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	GET_FLOAT_WORD(is, y);
	SET_FLOAT_WORD(y1, is & 0xfffff000);
	p_l = (y-y1)*t1 + y*t2;
	p_h = y1*t1;
	z = p_l + p_h;
	GET_FLOAT_WORD(j, z);
	if (j > 0x43000000)          /* if z > 128 */
		return sn*huge*huge;  /* overflow */
	else if (j == 0x43000000) {  /* if z == 128 */
		if (p_l + ovt > z - p_h)
			return sn*huge*huge;  /* overflow */
	} else if ((j&0x7fffffff) > 0x43160000)  /* z < -150 */ // FIXME: check should be  (uint32_t)j > 0xc3160000
		return sn*tiny*tiny;  /* underflow */
	else if (j == 0xc3160000) {  /* z == -150 */
		if (p_l <= z-p_h)
			return sn*tiny*tiny;  /* underflow */
	}
	/*
	 * compute 2**(p_h+p_l)
	 */
	i = j & 0x7fffffff;
	k = (i>>23) - 0x7f;
	n = 0;
	if (i > 0x3f000000) {   /* if |z| > 0.5, set n = [z+0.5] */
		n = j + (0x00800000>>(k+1));
		k = ((n&0x7fffffff)>>23) - 0x7f;  /* new k for n */
		SET_FLOAT_WORD(t, n & ~(0x007fffff>>k));
		n = ((n&0x007fffff)|0x00800000)>>(23-k);
		if (j < 0)
			n = -n;
		p_h -= t;
	}
	t = p_l + p_h;
	GET_FLOAT_WORD(is, t);
	SET_FLOAT_WORD(t, is & 0xffff8000);
	u = t*lg2_h;
	v = (p_l-(t-p_h))*lg2 + t*lg2_l;
	z = u + v;
	w = v - (z - u);
	t = z*z;
	t1 = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r = (z*t1)/(t1-2.0f) - (w+z*w);
	z = 1.0f - (r - z);
	GET_FLOAT_WORD(j, z);
	j += n<<23;
	if ((j>>23) <= 0)  /* subnormal output */
		z = scalbnf(z, n);
	else
		SET_FLOAT_WORD(z, j);
	return sn*z;
}

static inline float babl_powf (float x, float y)
{
  if (x > 0.0f)
    return expf (y * logf (x));
  return 0.0f;
}

static inline float _babl_trc_gamma_to_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  if (value >= trc->poly_gamma_to_linear_x0 &&
      value <= trc->poly_gamma_to_linear_x1)
    {
      return babl_polynomial_eval (&trc->poly_gamma_to_linear, value);
    }
  else
    {
      return babl_powf (value, trc->gamma);
    }
}

static inline float _babl_trc_gamma_from_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  if (value >= trc->poly_gamma_from_linear_x0 &&
      value <= trc->poly_gamma_from_linear_x1)
    {
      return babl_polynomial_eval (&trc->poly_gamma_from_linear, value);
    }
  else
    {
      return babl_powf (value, trc->rgamma);
    }
}

static inline void _babl_trc_gamma_to_linear_buf (const Babl *trc_, const float *in, float *out, int in_gap, int out_gap, int components, int count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = _babl_trc_gamma_to_linear (trc_, in[in_gap *i + c]);
}

static inline void _babl_trc_gamma_from_linear_buf (const Babl *trc_, const float *in, float *out, int in_gap, int out_gap, int components, int count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = _babl_trc_gamma_from_linear (trc_, in[in_gap *i + c]);
}

static inline float _babl_trc_formula_srgb_from_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];
  float d = trc->lut[4];
  if (x > c * d)  // XXX: verify that this math is the correct inverse
  {
    float v = _babl_trc_gamma_from_linear ((Babl *) trc, x);
    v = (v-b)/a;
    if (v < 0.0 || v >= 0.0)
      return v;
    return 0.0;
  }
  if (c > 0.0)
    return x / c;
  return 0.0;
}

static inline float _babl_trc_formula_srgb_to_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];
  float d = trc->lut[4];

  if (x >= d)
  {
    return _babl_trc_gamma_to_linear ((Babl *) trc, a * x + b);
  }
  return c * x;
}

static inline float _babl_trc_srgb_to_linear (const Babl *trc_, float value)
{
  return babl_gamma_2_2_to_linearf (value);
}

static inline float _babl_trc_srgb_from_linear (const Babl *trc_, float value)
{
  return babl_linear_to_gamma_2_2f (value);
}

static inline void _babl_trc_srgb_to_linear_buf (const Babl *trc_, const float *in, float *out, int in_gap, int out_gap, int components, int count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c++)
      out[out_gap * i + c] = babl_gamma_2_2_to_linearf (in[in_gap * i + c]);
}

static inline void _babl_trc_srgb_from_linear_buf (const Babl *trc_,
                                                   const float *in, float *out,
                                                   int in_gap, int out_gap,
                                                   int components,
                                                   int count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c++)
      out[out_gap * i + c] = babl_linear_to_gamma_2_2f (in[in_gap * i + c]);
}

static inline void _babl_trc_to_linear_buf_generic (const Babl *trc_, const float *in, float *out, int in_gap, int out_gap, int components, int count)
{
  int i, c;
  BablTRC *trc = (void*)trc_;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = trc->fun_to_linear (trc_, in[in_gap * i + c]);
}

static inline void _babl_trc_from_linear_buf_generic (const Babl *trc_,
                                                      const float *in, float *out,
                                                      int in_gap, int out_gap,
                                                      int components,
                                                      int count)
{
  int i, c;
  BablTRC *trc = (void*)trc_;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = trc->fun_from_linear (trc_, in[in_gap * i + c]);
}

static inline void _babl_trc_linear_buf (const Babl *trc_,
                                         const float *in, float *out,
                                         int in_gap, int out_gap,
                                         int components,
                                         int count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[i * out_gap + c] = in[i * in_gap + c];
}


const Babl *
babl_trc (const char *name)
{
  int i;
  for (i = 0; trc_db[i].instance.class_type; i++)
    if (!strcmp (trc_db[i].instance.name, name))
    {
      return (Babl*)&trc_db[i];
    }
  babl_log("failed to find trc '%s'\n", name);
  return NULL;
}

const Babl *
babl_trc_new (const char *name,
              BablTRCType type,
              double      gamma,
              int         n_lut,
              float      *lut)
{
  int i=0;
  static BablTRC trc;
  trc.instance.class_type = BABL_TRC;
  trc.instance.id         = 0;
  trc.type = type;
  trc.gamma  = gamma > 0.0    ? gamma       : 0.0;
  trc.rgamma = gamma > 0.0001 ? 1.0 / gamma : 0.0;

  if (n_lut )
  {
    for (i = 0; trc_db[i].instance.class_type; i++)
    {
    if ( trc_db[i].lut_size == n_lut &&
         (memcmp (trc_db[i].lut, lut, sizeof (float) * n_lut)==0)
       )
      {
        return (void*)&trc_db[i];
      }
    }
  }
  else
  for (i = 0; trc_db[i].instance.class_type; i++)
  {
    int offset = ((char*)&trc_db[i].type) - (char*)(&trc_db[i]);
    int size   = ((char*)&trc_db[i].gamma + sizeof(double)) - ((char*)&trc_db[i].type);

    if (memcmp ((char*)(&trc_db[i]) + offset, ((char*)&trc) + offset, size)==0)
      {
        return (void*)&trc_db[i];
      }
  }
  if (i >= MAX_TRCS-1)
  {
    babl_log ("too many BablTRCs");
    return NULL;
  }
  trc_db[i]=trc;
  trc_db[i].instance.name = trc_db[i].name;
  if (name)
    sprintf (trc_db[i].name, "%s", name);
  else if (n_lut)
    sprintf (trc_db[i].name, "lut-trc");
  else
    sprintf (trc_db[i].name, "trc-%i-%f", type, gamma);

  if (n_lut)
  {
    int j;
    trc_db[i].lut_size = n_lut;
    trc_db[i].lut = babl_calloc (sizeof (float), n_lut);
    memcpy (trc_db[i].lut, lut, sizeof (float) * n_lut);
    trc_db[i].inv_lut = babl_calloc (sizeof (float), n_lut);

    for (j = 0; j < n_lut; j++)
    {
      int k;
      double min = 0.0;
      double max = 1.0;
      for (k = 0; k < 16; k++)
      {
        double guess = (min + max) / 2;
        float reversed_index = babl_trc_lut_to_linear (BABL(&trc_db[i]), guess) * (n_lut-1.0);

        if (reversed_index < j)
        {
          min = guess;
        }
        else if (reversed_index > j)
        {
          max = guess;
        }
      }
      trc_db[i].inv_lut[j] = (min + max) / 2;
    }
  }

  trc_db[i].fun_to_linear_buf = _babl_trc_to_linear_buf_generic;
  trc_db[i].fun_from_linear_buf = _babl_trc_from_linear_buf_generic;

  switch (trc_db[i].type)
  {
    case BABL_TRC_LINEAR:
      trc_db[i].fun_to_linear = _babl_trc_linear;
      trc_db[i].fun_from_linear = _babl_trc_linear;
      trc_db[i].fun_from_linear_buf = _babl_trc_linear_buf;
      trc_db[i].fun_to_linear_buf = _babl_trc_linear_buf;
      break;
    case BABL_TRC_FORMULA_GAMMA:
      trc_db[i].fun_to_linear = _babl_trc_gamma_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_gamma_from_linear;
      trc_db[i].fun_to_linear_buf = _babl_trc_gamma_to_linear_buf;
      trc_db[i].fun_from_linear_buf = _babl_trc_gamma_from_linear_buf;

      trc_db[i].poly_gamma_to_linear_x0 = POLY_GAMMA_X0;
      trc_db[i].poly_gamma_to_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_to_linear,
                                         trc_db[i].gamma,
                                         trc_db[i].poly_gamma_to_linear_x0,
                                         trc_db[i].poly_gamma_to_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);

      trc_db[i].poly_gamma_from_linear_x0 = POLY_GAMMA_X0;
      trc_db[i].poly_gamma_from_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_from_linear,
                                         trc_db[i].rgamma,
                                         trc_db[i].poly_gamma_from_linear_x0,
                                         trc_db[i].poly_gamma_from_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);
      break;
    case BABL_TRC_FORMULA_SRGB:
      trc_db[i].lut = babl_calloc (sizeof (float), 5);
      {
        int j;
        for (j = 0; j < 5; j++)
          trc_db[i].lut[j] = lut[j];
      }
      trc_db[i].fun_to_linear = _babl_trc_formula_srgb_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_formula_srgb_from_linear;

      trc_db[i].poly_gamma_to_linear_x0 = lut[4];
      trc_db[i].poly_gamma_to_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_to_linear,
                                         trc_db[i].gamma,
                                         trc_db[i].poly_gamma_to_linear_x0,
                                         trc_db[i].poly_gamma_to_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);

      trc_db[i].poly_gamma_from_linear_x0 = lut[3] * lut[4];
      trc_db[i].poly_gamma_from_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_from_linear,
                                         trc_db[i].rgamma,
                                         trc_db[i].poly_gamma_from_linear_x0,
                                         trc_db[i].poly_gamma_from_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);
      break;
    case BABL_TRC_SRGB:
      trc_db[i].fun_to_linear = _babl_trc_srgb_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_srgb_from_linear;
      trc_db[i].fun_from_linear_buf = _babl_trc_srgb_from_linear_buf;
      trc_db[i].fun_to_linear_buf = _babl_trc_srgb_to_linear_buf;
      break;
    case BABL_TRC_LUT:
      trc_db[i].fun_to_linear = babl_trc_lut_to_linear;
      trc_db[i].fun_from_linear = babl_trc_lut_from_linear;
      break;
  }
  return (Babl*)&trc_db[i];
}

const Babl * babl_trc_lut (const char *name, int n, float *entries)
{
  return babl_trc_new (name, BABL_TRC_LUT, 0, n, entries);
}

void
babl_trc_class_for_each (BablEachFunction each_fun,
                           void            *user_data)
{
  int i=0;
  for (i = 0; trc_db[i].instance.class_type; i++)
    if (each_fun (BABL (&trc_db[i]), user_data))
      return;
}

const Babl *
babl_trc_formula_srgb (double g, double a, double b, double c, double d)
{
  char name[128];
  int i;
  float params[5]={g, a, b, c, d};

  if (fabs (g - 2.40)     < 0.01 &&
      fabs (a - 26214)    < 0.01 &&
      fabs (b - 0.947875) < 0.01 &&
      fabs (c - (-3417))  < 0.01)
    return babl_trc ("sRGB");

  sprintf (name, "%.6f %.6f %.4f %.4f %.4f", g, a, b, c, d);
  for (i = 0; name[i]; i++)
    if (name[i] == ',') name[i] = '.';
  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_SRGB, g, 0, params);
}

const Babl *
babl_trc_gamma (double gamma)
{
  char name[32];
  int i;
  if (fabs (gamma - 1.0) < 0.01)
     return babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);

  sprintf (name, "%.6f", gamma);
  for (i = 0; name[i]; i++)
    if (name[i] == ',') name[i] = '.';
  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_GAMMA, gamma, 0, NULL);
}

void
babl_trc_class_init (void)
{
  babl_trc_new ("sRGB",  BABL_TRC_SRGB, 2.2, 0, NULL);
  babl_trc_gamma (2.2);
  babl_trc_gamma (1.8);
  babl_trc_gamma (1.0);
  babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);
}

#if 0
float babl_trc_from_linear (const Babl *trc_, float value)
{
  return babl_trc_from_linear (trc_, value);
}

float babl_trc_to_linear (const Babl *trc_, float value)
{
  return babl_trc_to_linear (trc_, value);
}
#endif

const Babl *babl_trc_lut_find (float *lut, int lut_size)
{
  int i;
  int match = 1;

  /* look for linear match */
  for (i = 0; match && i < lut_size; i++)
    if (fabs (lut[i] - i / (lut_size-1.0)) > 0.015)
      match = 0;
  if (match)
    return babl_trc_gamma (1.0);

  /* look for 2.2 match: */
  match = 1;
  if (lut_size > 1024)
  {
  for (i = 0; match && i < lut_size; i++)
  {
#if 0
    fprintf (stderr, "%i %f %f\n", i,
                   lut[i],
                   pow ((i / (lut_size-1.0)), 2.2));
#endif
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 2.2)) > 0.0001)
      match = 0;
  }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 2.2)) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc_gamma(2.2);


  /* look for 1.8 match: */
  match = 1;
  if (lut_size > 1024)
  {
  for (i = 0; match && i < lut_size; i++)
  {
#if 0
    fprintf (stderr, "%i %f %f\n", i,
                   lut[i],
                   pow ((i / (lut_size-1.0)), 1.8));
#endif
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 1.8)) > 0.0001)
      match = 0;
  }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 1.8)) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc_gamma(2.2);


  /* look for sRGB match: */
  match = 1;
  if (lut_size > 1024)
  {
  for (i = 0; match && i < lut_size; i++)
  {
    if (fabs (lut[i] - gamma_2_2_to_linear (i / (lut_size-1.0))) > 0.0001)
      match = 0;
  }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
      if (fabs (lut[i] - gamma_2_2_to_linear (i / (lut_size-1.0))) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc ("sRGB");

  return NULL;
}

