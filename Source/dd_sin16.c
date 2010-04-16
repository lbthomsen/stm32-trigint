#include "dd_sin16.h"
#include <stdbool.h>

#include <math.h>
#include <stdio.h>

/*
 * http://www.dattalo.com/technical/software/pic/picsine.html
 */

/*
 * dd_sin16_angle_t is a 14-bit angle, 0 - 0x3FFFF
 *
 * xxQQTTTT IIIIIIII
 * QQ - Quadrant, 00 = quandrant 1, 01 = quadrant 2, etc.
 * TTTT - Table index into sine16_table
 * IIIIIIII - Interpolation between successive entries in the table 
 */

#define SINE_INDEX_WIDTH 4
#define SINE_INTERP_WIDTH 8

#if (SINE_INDEX_WIDTH + SINE_INTERP_WIDTH > 12)
# error Invalid sine widths
#endif

#define SINE_INDEX_OFFSET (12 - SINE_INDEX_WIDTH)
#define SINE_INTERP_OFFSET (SINE_INDEX_OFFSET - SINE_INTERP_WIDTH)

#define SINE_TABLE_SIZE (1 << SINE_INDEX_WIDTH)
// Table of the first quadrant values.  Use + 1 to store the first value of
// the second quadrant, hence we're storing 0 <= degrees <= 90.
static const int16_t sine16_table[SINE_TABLE_SIZE + 1] = {
        0,  3211,  6392,  9511, 12539, 15446, 18204, 20787,
    23169, 25329, 27244, 28897, 30272, 31356, 32137, 32609,
    32767
};

#define SINE_TABLE_COUNT (sizeof(sine16_table)/sizeof(*sine16_table))


#define BITS(_V_, _W_, _O_) (((_V_) >> (_O_)) & ((1 << (_W_)) - 1))


int16_t dd_sin16(dd_sin16_angle_t angle)
{
#if 0
	uint8_t interp = (angle >> 0) & 0xff;
	uint8_t index = (angle >> 8) & 0x0f;
	uint8_t quadrant = (angle >> 12) & 0x03;
#else
	int32_t interp = BITS(angle, SINE_INTERP_WIDTH, SINE_INTERP_OFFSET);
	uint8_t o_index = BITS(angle, SINE_INDEX_WIDTH, SINE_INDEX_OFFSET);
	uint8_t quadrant = BITS(angle, 2, 12);
#endif
	bool isOdd = (quadrant & 0x01) == 0;
	bool isNeg = (quadrant & 0x02) != 0;
	uint8_t index = o_index;
	if (!isOdd) {
		index = SINE_TABLE_SIZE - 1 - o_index;
	}
	
	
	int32_t x1 = sine16_table[index];
	int32_t x2 = sine16_table[index+1];
#if 0
	int32_t diff = (x2-x1);
	int32_t step = diff * (int32_t)interp;
	int32_t shift = step >> SINE_INTERP_WIDTH;
	int32_t sine = x1 + shift;
#elif 0
	int16_t sine = x1 + (((int32_t)(x2-x1)) * (int32_t)interp) >> SINE_INTERP_WIDTH;
#else
	int32_t shift = ((x2-x1) * interp) >> SINE_INTERP_WIDTH;
	int16_t sine;
	if (isOdd) {
		sine = x1 + shift;
	}
	else {
		sine = x2 - shift;
	}
	if (isNeg) {
		sine *= -1;
	}
#endif
	return sine;
}

#pragma mark -
#pragma mark Conversion Routines

dd_sin16_angle_t dd_sin16_degrees_to_angle_d(double degrees)
{
	dd_sin16_angle_t angle = (dd_sin16_angle_t)((degrees * 0x4000) / 360.0);
	return angle;
}

dd_sin16_angle_t dd_sin16_degrees_to_angle_i(int degrees)
{
	dd_sin16_angle_t angle = (dd_sin16_angle_t)((degrees * 0x4000) / 360);
	return angle;
}

double dd_sin16_angle_to_degrees_d(dd_sin16_angle_t angle)
{
    double angle_d = angle;
    double degrees = angle_d * 360 / 0x4000;
    return degrees;
}

int dd_sin16_angle_to_degrees_i(dd_sin16_angle_t angle)
{
    int angle_i = angle;
    int degrees = angle_i * 360 / 0x4000;
    return degrees;
}

dd_sin16_angle_t dd_sin16_radians_to_angle_d(double radians)
{
	dd_sin16_angle_t angle = (dd_sin16_angle_t)((radians * 0x4000) / (2*M_PI));
	return angle;
}

double dd_sin16_angle_to_radians_d(dd_sin16_angle_t angle)
{
    double angle_d = angle;
    double radians = (angle_d * 2.0 * M_PI) / 0x4000;
    return radians;
}

#pragma mark -

void dd_sin16_table()
{
    char * sep = "";
	for (int i = 0; i < SINE_TABLE_COUNT; i++) {
		double radians = i * M_PI_2 / SINE_TABLE_SIZE;
		double sinValue = 32767.0 * sin(radians);
		int16_t tableValue = sinValue;
		printf("%s%5d", sep, tableValue);
        if (((i+1) % 8) == 0) {
            sep = ",\n";
        } else {
            sep = ", ";
        }

	}
}