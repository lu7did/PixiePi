/*

    1/7/2017, Rick Koch / N1GP adapted from midicw.c
    found here: https://github.com/recri/keyer
    credits below.

*/

/*
  Copyright (C) 2011, 2012 by Roger E Critchlow Jr, Santa Fe, NM, USA.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/**
* @brief Functions to allow windowing on the signal
* @author Frank Brickle, AB2KT and Bob McGwier, N4HY

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006,2007 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Implemented from code by Bill Schottstaedt of Snd Editor at CCRMA
Doxygen comments added by Dave Larsen, KV0S

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 7 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@gmail.com

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

/** shamelessly stolen from Bill Schottstaedt's clm.c
* made worse in the process, but enough for our purposes here
*/


/** mostly taken from
 *    Fredric J. Harris, "On the Use of Windows for Harmonic Analysis with the
 *    Discrete Fourier Transform," Proceedings of the IEEE, Vol. 66, No. 1,
 *    January 1978.
 *    Albert H. Nuttall, "Some Windows with Very Good Sidelobe Behaviour",
 *    IEEE Transactions of Acoustics, Speech, and Signal Processing, Vol. ASSP-29,
 *    No. 1, February 1981, pp 84-91
 *
 * JOS had slightly different numbers for the Blackman-Harris windows.
 */

#ifndef KEYED_TONE_H
#define KEYED_TONE_H
/*
** keyed tone
** generates a tone with specified frequency and gain
**
** There are several possible keying envelopes
** sine uses 1/4 of a sine wave: sin(0) .. sin(pi/2); or sin(pi/2) .. sin(pi)
** raised cosine uses 1/2 of a sine wave: 1+cos(-pi) .. 1+cos(0); or 1+cos(0) .. 1+cos(pi)
** blackman-harris uses halves of the blackman-harris window function
** basically, all of the window functions are possibilities, but once you think about it
** you go with blackman-harris because it minimises key clicks.
**
** one might argue to use the window function itself as the entire keying envelope,
** stretching it to cover the entire dit or dah, rather than separating the attack,
** sustain, and decay phases and stitching them together.  this would mean that you
** know the length of the pulse when you start it, the rise and fall would be 1/2 the
** pulse length, and the keyed envelope would be smooth.
**
** but you don't usually know the pulse length until key up.
*/

extern "C" {
#include <string.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

int keyed_tone_start(long volume, double freq, int envelope);
extern int keyed_tone_mute;
void keyed_tone_close();

static const float pi = 3.14159265358979323846f;		/* pi */
static const float two_pi = 2*3.14159265358979323846f;		/* 2*pi */
static const double dpi = 3.14159265358979323846;		/* pi */
static const double dtwo_pi = 2*3.14159265358979323846;		/* 2*pi */

static float sqrf(float x) {
    return (x * x);
}
static double sqr(double x) {
    return (x * x);
}

/*
** these are banal
*/
static float minf(float a, float b) {
    return a < b ? a : b;
}
static float maxf(float a, float b) {
    return a > b ? a : b;
}
static float squaref(float x) {
    return x * x;
}

static double square(double x) {
    return x * x;
}

#if ! defined(OSCILLATOR_F) && ! defined(OSCILLATOR_T) && ! defined(OSCILLATOR_Z)
#error "oscillator.h has no implementation selected"
#endif

#ifdef OSCILLATOR_D
typedef double ofloat;
typedef double complex ocomplex;
#define osqrt(x) sqrt(x)
#define osquare(x) square(x)
#define otan(x) tan(x)
#define ocos(x) cos(x)
#define osin(x) sin(x)
#define opi	dpi
#define otwo_pi dtwo_pi
static const double oone = 1.0;
#else
typedef float ofloat;
typedef float complex ocomplex;
#define osqrt(x) sqrtf(x)
#define osquare(x) squaref(x)
#define otan(x) tanf(x)
#define ocos(x) cosf(x)
#define osin(x) sinf(x)
#define opi	pi
#define otwo_pi two_pi
static const float oone = 1.0f;
#endif

#ifdef OSCILLATOR_F
/*
** oscillator - a recursive filter
** in its original form it only generates
** positive frequencies.
*/
typedef struct {
    ofloat xi, c, x, y;
    ocomplex (*finish)(ofloat x, ofloat y);
} oscillator_t;

static ocomplex oscillator_finish_positive_frequency(ofloat x, ofloat y) {
    return x + I*y;
}
static ocomplex oscillator_finish_negative_frequency(ofloat x, ofloat y) {
    return x - I*y;
}
static void oscillator_set_hertz(oscillator_t *o, float hertz, int samples_per_second) {
    ofloat current_xi = o->xi;
    ofloat wps = hertz / samples_per_second;
    ofloat rps = wps * otwo_pi;
    o->c = osqrt(oone / (oone + osquare(otan(rps))));
    o->xi = osqrt((oone - o->c) / (oone + o->c));
    o->x *=  o->xi / current_xi;
    o->finish = (hertz > 0) ? oscillator_finish_positive_frequency : oscillator_finish_negative_frequency;
}

static void oscillator_set_phase(oscillator_t *o, float radians) {
    o->x = ocos(radians) * o->xi;
    o->y = osin(radians);
}

static float complex oscillator_process(oscillator_t *o) {
    ofloat t = (o->x + o->y) * o->c;
    ofloat nx = t-o->y;
    ofloat ny = t+o->x;
    ofloat x = (o->x = nx) / o->xi; /* better as multiply by inverse? */
    ofloat y = o->y = ny;
    return o->finish(x, y);
}

#endif

#ifdef OSCILLATOR_T
/*
** oscillator - a trigonometric function
*/
typedef struct {
    ofloat phase, dphase;
} oscillator_t;

static void oscillator_set_hertz(oscillator_t *o, float hertz, int samples_per_second) {
    o->dphase = otwo_pi * hertz / samples_per_second;
}

static void oscillator_set_phase(oscillator_t *o, float radians) {
    o->phase = radians;
}

static float complex oscillator_process(oscillator_t *o) {
    o->phase += o->dphase;
    while (o->phase > otwo_pi) o->phase -= otwo_pi;
    while (o->phase < -otwo_pi) o->phase += otwo_pi;
    return ocos(o->phase) + I * osin(o->phase);
}
#endif

#ifdef OSCILLATOR_Z
/*
** oscillator - a complex rotor
*/
typedef struct {
    ocomplex phase, dphase;
} oscillator_t;

static void oscillator_set_hertz(oscillator_t *o, float hertz, int samples_per_second) {
    ofloat dradians = otwo_pi * hertz / samples_per_second;
    o->dphase = ocos(dradians) + I * osin(dradians);
}

static void oscillator_set_phase(oscillator_t *o, float radians) {
    o->phase = ocos(radians) + I * osin(radians);
}

static float complex oscillator_process(oscillator_t *o) {
    return o->phase *= o->dphase;
}

#endif

/*
** code common to all implementations.
*/

static void *oscillator_init(oscillator_t *o, float hertz, float radians, int samples_per_second) {
    oscillator_set_hertz(o, hertz, samples_per_second);
    oscillator_set_phase(o, radians);
    return o;
}

static void oscillator_update(oscillator_t *o, float hertz, int samples_per_second) {
    oscillator_set_hertz(o, hertz, samples_per_second);
}

static void oscillator_reset(oscillator_t *o) {
    oscillator_set_phase(o, 0.0f);
}

typedef enum {
    WINDOW_RECTANGULAR = 0,
    WINDOW_HANNING = 1,		/* Hann */
    WINDOW_WELCH = 2,
    WINDOW_PARZEN = 3,
    WINDOW_BARTLETT = 4,
    WINDOW_HAMMING = 5,
    WINDOW_BLACKMAN2 = 6,
    WINDOW_BLACKMAN3 = 7,
    WINDOW_BLACKMAN4 = 8,
    WINDOW_EXPONENTIAL = 9,
    WINDOW_RIEMANN = 10,
    WINDOW_BLACKMAN_HARRIS = 11,
    WINDOW_BLACKMAN_NUTTALL = 12,
    WINDOW_NUTTALL = 13,
    WINDOW_FLAT_TOP = 14,
    WINDOW_TUKEY = 15,
    WINDOW_COSINE = 16,
    WINDOW_LANCZOS = 17,
    WINDOW_TRIANGULAR = 18,
    WINDOW_GAUSSIAN = 19,
    WINDOW_BARTLETT_HANN = 20,
    WINDOW_KAISER = 21
} window_type_t;

static char *window_names[] = {
    "rectangular",
    "hanning",
    "welch",
    "parzen",
    "bartlett",
    "hamming",
    "blackman2",
    "blackman3",
    "blackman4",
    "exponential",
    "riemann",
    "blackman-harris",
    "blackman-nuttall",
    "nuttall",
    "flat-top",
    "tukey",
    "cosine",
    "lanczos",
    "triangular",
    "gaussian",
    "bartlett-hann",
    "kaiser",
    NULL
};

/* -------------------------------------------------------------------------- */
/** @brief Function to make the window
*
* @param type -- uses window_types_t
* @param size -- size of window
* @param window -- data
* @return void
*/
/* ---------------------------------------------------------------------------- */
static float window_get(const window_type_t type, const int size, int k) {
    int i, j;
    switch (type) {
    case WINDOW_RECTANGULAR:
        return 1.0;
    case WINDOW_HANNING: {	/* Hann would be more accurate */
        const int midn = size >> 1;
        if (k > midn) k = (size-1) - k;
        return 0.5 - 0.5 * cos(k * dtwo_pi / (size-1));
    }
    case WINDOW_WELCH: {
        const int midn = size >> 1;
        const int midm1 = (size - 1) / 2;
        const int midp1 = (size + 1) / 2;
        if (k > midn) k = size-1 - k;
        return 1.0 - sqr( (k - midm1) / (double) midp1);
    }
    case WINDOW_PARZEN: {
        const int midn = size >> 1;
        const int midm1 = (size - 1) / 2;
        const int midp1 = (size + 1) / 2;
        if (k > midn) k = size-1 - k;
        return 1.0 - fabs( (k - midm1) / (double) midp1);
    }
    case WINDOW_BARTLETT: {
        const int midn = size >> 1;
        if (k > midn) k = size-1 - k;
        return k / (double)midn;
    }
    case WINDOW_HAMMING: {
        const int midn = size >> 1;
        if (k > midn) k = size-1 - k;
        return 0.54 - 0.46 * cos(k * dtwo_pi / (size-1));
    }
    case WINDOW_BLACKMAN2: {	/* using Chebyshev polynomial equivalents here */
        const int midn = size >> 1;
        if (k > midn) k = size-1 - k;
        double cx = cos(k * dtwo_pi / size);
        return .34401 + (cx * (-.49755 + (cx * .15844)));
    }
    case WINDOW_BLACKMAN3: {
        const int midn = size >> 1;
        if (k > midn) k = size-1 - k;
        double cx = cos(k * dtwo_pi / size);
        return .21747 + (cx * (-.45325 + (cx * (.28256 - (cx * .04672)))));
    }
    case WINDOW_BLACKMAN4: {
        const int midn = size >> 1;
        if (k > midn) k = size-1 - k;
        double cx = cos(k * dtwo_pi / size);
        return .084037 + (cx * (-.29145 + (cx * (.375696 + (cx * (-.20762 + (cx * .041194)))))));
    }
    case WINDOW_EXPONENTIAL: {
        const int midn = size >> 1;
        const double expn = log(2.0) / midn + 1.0;
        double expsum = 1.0;
        for (i = 0, j = size - 1; i <= midn; i++, j--) {
            if (i == k || j == k)
                return expsum - 1.0;
            expsum *= expn;
        }
        break;
    }
    case WINDOW_RIEMANN: {
        const int midn = size >> 1;
        if (midn == k) return 1.0;
        if (k > midn) k = size-1 - k;
        const double cx = (midn - k) * dtwo_pi / size;
        return sin(cx) / cx;
    }
    case WINDOW_BLACKMAN_HARRIS: {
        // corrected per wikipedia
        const double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;
        const double arg = k * dtwo_pi / (size - 1);
        return a0 - a1 * cos(arg) + a2 * cos(2 * arg) - a3 * cos(3 * arg);
    }
    case WINDOW_BLACKMAN_NUTTALL: {
        // corrected and renamed per wikipedia
        const double a0 = 0.3635819, a1 = 0.4891775, a2 = 0.1365995, a3 = 0.0106411;
        const double arg = k * dtwo_pi / (size - 1);
        return a0 - a1 * cos(arg) + a2 * cos(2 * arg) - a3 * cos(3 * arg);
    }
    case WINDOW_NUTTALL: {
        // wikipedia's version
        const double a0 = 0.355768, a1 = 0.487396, a2 = 0.144232, a3 = 0.012604;
        const double arg = k * dtwo_pi / (size - 1);
        return a0 - a1 * cos(arg) + a2 * cos(2 * arg) - a3 * cos(3 * arg);
    }
    case WINDOW_FLAT_TOP: {
        const double a0 = 1.0, a1 = 1.93, a2 = 1.29, a3 = 0.388, a4 = 0.032;
        const double arg = k * dtwo_pi / (size - 1);
        return a0 - a1 * cos(arg) + a2 * cos(2 * arg) - a3 * cos(3 * arg) + a4 * cos(4 * arg);

    }
    case WINDOW_TUKEY: {
        // Tukey window is an interpolation between a Hann and a rectangular window
        // parameterized by alpha, somewhat like a raised cosine keyed tone
        return 0;
    }
    case WINDOW_COSINE: {
        // also known as the sine window
        return sin(pi*k / (size-1));
    }
    case WINDOW_LANCZOS: {
        return 0;// sinc(2*k/(size-1)), normalized sinc(x) = sin(pi x) / (pi x), sinc(0) == 1
    }
    case WINDOW_TRIANGULAR: {
        return 2.0 / (size+1) * ((size+1)/2.0 - fabs(k-(size-1)/2.0));
    }
    case WINDOW_GAUSSIAN: {
        // gaussian parameterized by sigma <= 0.5
        const double sigma = 0.5;
        return exp(-0.5 * pow((k - (size-1) / 2.0) / (sigma * (size-1) / 2.0), 2));
    }
    case WINDOW_BARTLETT_HANN: {
        return 0;
    }
    case WINDOW_KAISER: {
        return 0;
    }
    }
    return 1.0 / 0.0;
}

/*
** Blackman Harris attack/decay ramp
** uses 1/2 of the Blackman Harris window function
** from sin(0 .. 0.5) for ramp on
** from sin(0.5 .. 1.0) for ramp off
*/
typedef struct {
    int do_rise;			/* rising or falling ramp */
    int target;			/* sample length of ramp */
    int current;			/* current sample point in ramp */
    float *ramp;			/* ramp values */
} ramp_t;

static void ramp_update(ramp_t *r, float ms, int samples_per_second) {
    int i;
    r->target = samples_per_second * (ms / 1000.0f);
    if (r->target < 1) r->target = 1;
    if ((r->target & 1) == 0) r->target += 1;
    r->current = 0;
    r->ramp = realloc(r->ramp, r->target*sizeof(float));
    for (i = 0; i < r->target; i += 1)
        r->ramp[i] = window_get(WINDOW_BLACKMAN_HARRIS, 2*r->target-1, i);
}

static void ramp_init(ramp_t *r, float ms, int samples_per_second) {
    r->ramp = NULL;
    ramp_update(r, ms, samples_per_second);
}

static void ramp_start_rise(ramp_t *r) {
    r->do_rise = 1;
    r->current = 0;
}

static void ramp_start_fall(ramp_t *r) {
    r->do_rise = 0;
    r->current = 0;
}

static float ramp_next(ramp_t *r) {
    r->current += 1;
    float v = r->current < r->target ? r->ramp[r->current] : 1;
    return r->do_rise ? v : 1-v;
}

static int ramp_done(ramp_t *r) {
    return r->current >= r->target;
}

static void ramp_free(ramp_t *r) {
    if (r->ramp != NULL) free(r->ramp);
}

#define KEYED_TONE_OFF	0	/* note is not sounding */
#define KEYED_TONE_RISE	1	/* note is ramping up to full level */
#define KEYED_TONE_ON	2	/* note is sounding full level */
#define KEYED_TONE_FALL	3	/* note is ramping down to off */

typedef struct {
    int state;			/* state of cwtone */
    float gain;			/* target gain */
    oscillator_t tone;		/* tone oscillator */
    ramp_t rise;			/* tone on ramp */
    ramp_t fall;			/* tone off ramp */
} keyed_tone_t;

static void keyed_tone_update(keyed_tone_t *p, float gain_dB, float freq, float rise, float fall, unsigned sample_rate) {
    p->gain = powf(10.0f, gain_dB / 20.0f);
    oscillator_update(&p->tone, freq, sample_rate);
    ramp_update(&p->rise, rise, sample_rate);
    ramp_update(&p->fall, fall, sample_rate);
}

static void *keyed_tone_init(keyed_tone_t *p, float gain_dB, float freq, float rise, float fall, unsigned sample_rate) {
    p->state = KEYED_TONE_OFF;
    p->gain = powf(10.0f, gain_dB / 20.0f);
    oscillator_init(&p->tone, freq, 0.0f, sample_rate);
    ramp_init(&p->rise, rise, sample_rate);
    ramp_init(&p->fall, fall, sample_rate);
    return p;
}

static void keyed_tone_on(keyed_tone_t *p) {
    p->state = KEYED_TONE_RISE;
    ramp_start_rise(&p->rise);
}

static void keyed_tone_off(keyed_tone_t *p) {
    p->state = KEYED_TONE_FALL;
    ramp_start_fall(&p->fall);
}

static float _Complex keyed_tone_process(keyed_tone_t *p) {
    float scale = p->gain;
    switch (p->state) {
    case KEYED_TONE_OFF:	/* note is not sounding */
        scale = 0;
        break;
    case KEYED_TONE_RISE:	/* note is ramping up to full level */
        scale *= ramp_next(&p->rise);
        if (ramp_done(&p->rise))
            p->state = KEYED_TONE_ON;
        break;
    case KEYED_TONE_ON:	/* note is sounding full level */
        break;
    case KEYED_TONE_FALL:	/* note is ramping down to off */
        scale *= ramp_next(&p->fall);
        if (ramp_done(&p->fall))
            p->state = KEYED_TONE_OFF;
        break;
    }
    return scale * oscillator_process(&p->tone);
}
#endif
}
