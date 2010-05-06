//

#include "trigint_sin16.h"
#include <stdbool.h>

#include <math.h>
#include <stdio.h>


#define SINE_INDEX_WIDTH 4
#define SINE_INTERP_WIDTH 8

/*
 * Implementation based off of:
 * http://www.dattalo.com/technical/software/pic/picsine.html
 *
 * trigint_angle_t is a 14-bit angle, 0 - 0x3FFFF
 *
 * xxQQTTTT IIIIPPPP
 * Q - Quadrant, 00 = quandrant 1, 01 = quadrant 2, etc.
 * T - Table index into sine16_table, SINE_INDEX_WIDTH.
 * I - Interpolation between successive entries in the table, SINE_INTERP_WIDTH.
 * P - Phase accumalation, may be zero width. Used for rounding.
 */

#if (SINE_INDEX_WIDTH + SINE_INTERP_WIDTH > 12)
# error Invalid sine widths
#endif
 
#define SINE_INDEX_OFFSET (12 - SINE_INDEX_WIDTH)
#define SINE_INTERP_OFFSET (SINE_INDEX_OFFSET - SINE_INTERP_WIDTH)

/* Define a MAX macro if we don't already have one */
#ifndef MAX
# define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#if SINE_INTERP_OFFSET > 0
# define SINE_ROUNDING (1 << (SINE_INTERP_OFFSET-1))
#else
# define SINE_ROUNDING (0)
#endif

#define SINE_TABLE_SIZE (1 << SINE_INDEX_WIDTH)

#ifndef DD_SIN16_STATIC_TABLE
# define DD_SIN16_STATIC_TABLE 1
#endif

#if DD_SIN16_STATIC_TABLE

// Table of the first quadrant values.  Size is + 1 to store the first value of
// the second quadrant, hence we're storing 0 <= degrees <= 90.

static const int16_t dd_sine16_table[SINE_TABLE_SIZE + 1] = {
        0,  3211,  6392,  9511, 12539, 15446, 18204, 20787,
    23169, 25329, 27244, 28897, 30272, 31356, 32137, 32609,
    32767
};

#else

static int16_t dd_sine16_table[SINE_TABLE_SIZE + 1];

#endif

#define SINE_TABLE_COUNT (sizeof(dd_sine16_table)/sizeof(*dd_sine16_table))


#define BITS(_VALUE_, _WIDTH_, _BIT_) (((_VALUE_) >> (_BIT_)) & ((1 << (_WIDTH_)) - 1))


int trigint_sin16_table_size()
{
    return SINE_TABLE_COUNT;
}

inline int16_t trigint_sin16_table_lookup(int index)
{
    return dd_sine16_table[index];
}

int16_t dd_sin16(trigint_angle_t angle)
{
    angle += SINE_ROUNDING;
	int32_t interp = BITS(angle, SINE_INTERP_WIDTH, SINE_INTERP_OFFSET);
	uint8_t index = BITS(angle, SINE_INDEX_WIDTH, SINE_INDEX_OFFSET);
	uint8_t quadrant = BITS(angle, 2, 12);

	bool isOddQuadrant = (quadrant & 0x01) == 0;
	bool isNegativeQuadrant = (quadrant & 0x02) != 0;
    
	if (!isOddQuadrant) {
		index = SINE_TABLE_SIZE - 1 - index;
	}
	
    // Do calculations with 32 bits since the multiplication can overflow 16 bits
	int32_t x1 = trigint_sin16_table_lookup(index);
	int32_t x2 = trigint_sin16_table_lookup(index+1);
    int32_t approximation = ((x2-x1) * interp) >> SINE_INTERP_WIDTH;
    
	int16_t sine;
	if (isOddQuadrant) {
		sine = x1 + approximation;
	} else {
		sine = x2 - approximation;
	}
    
	if (isNegativeQuadrant) {
		sine *= -1;
	}

	return sine;
}

#pragma mark -
#pragma mark Conversion Routines

trigint_angle_t trigint_degrees_to_angle_d(double degrees)
{
	trigint_angle_t angle = (trigint_angle_t)((degrees * 0x4000) / 360.0);
	return angle;
}

trigint_angle_t trigint_degrees_to_angle_i(int degrees)
{
	trigint_angle_t angle = (trigint_angle_t)((degrees * 0x4000) / 360);
	return angle;
}

double trigint_angle_to_degrees_d(trigint_angle_t angle)
{
    double angle_d = angle;
    double degrees = angle_d * 360 / 0x4000;
    return degrees;
}

int trigint_angle_to_degrees_i(trigint_angle_t angle)
{
    int angle_i = angle;
    int degrees = angle_i * 360 / 0x4000;
    return degrees;
}

trigint_angle_t trigint_radians_to_angle_d(double radians)
{
	trigint_angle_t angle = (trigint_angle_t)((radians * 0x4000) / (2*M_PI));
	return angle;
}

double trigint_angle_to_radians_d(trigint_angle_t angle)
{
    double angle_d = angle;
    double radians = (angle_d * 2.0 * M_PI) / 0x4000;
    return radians;
}

#pragma mark -

#if !DD_SIN16_STATIC_TABLE

void trigint_sin16_init()
{
	for (int i = 0; i < SINE_TABLE_COUNT; i++) {
		double radians = i * M_PI_2 / SINE_TABLE_SIZE;
		double sinValue = 32767.0 * sin(radians);
		int16_t tableValue = round(sinValue);
        dd_sine16_table[i] = tableValue;
	}
}

#endif