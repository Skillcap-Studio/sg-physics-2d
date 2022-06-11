/*************************************************************************/
/* Copyright (c) 2021-2022 David Snopek                                  */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "sg_fixed_number_internal.h"

#include "../thirdparty/libfixmath/fixmath.h"

/**
 * Copied from https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_.28base_2.29
 * but modified to use 64-bit numbers.
 *
 * Like fix16_sqrt(), for negative numbers we return the negated square root of
 * the absolute value (ie. sqrt(-x) = -sqrt(x)).
 */
int64_t sg_sqrt_64(int64_t num) {
	if (num == 0) {
		return 0;
	}

	bool neg = num < 0;
	if (neg) {
		num = -num;
	}
	int64_t res = 0;
	int64_t bit = 1LL << 62;

	// Start bit at the highest power of four that's less than or equal to num.
	while (bit > num) {
		bit >>= 2;
	}

	while (bit != 0) {
		if (num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		}
		else {
			res >>= 1;
		}
		bit >>= 2;
	}

	return neg ? -res : res;
}

const fixed fixed::ZERO = fixed(0);
const fixed fixed::ONE  = fixed(65536);
const fixed fixed::HALF = fixed(32768);
const fixed fixed::TWO  = fixed(131072);
const fixed fixed::NEG_ONE = fixed(-65536);
const fixed fixed::PI = fixed(205887);
const fixed fixed::TAU = fixed(411774);
const fixed fixed::PI_DIV_2 = fixed(102943);
const fixed fixed::PI_DIV_4 = fixed(51472);
const fixed fixed::EPSILON = fixed(1);
const fixed fixed::ARITHMETIC_OVERFLOW = fixed(INT64_MIN);
const fixed fixed::DEG_180 = fixed(11796480);
const int64_t fixed::TENS[] { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };
const int fixed::TENS_SIZE = sizeof(TENS)/sizeof(TENS[0]);

// Adapted form the fpm library: https://github.com/MikeLankamp
// Copyright 2019 Mike Lankamp
// License: MIT
fixed fixed::sin() const {
    // This sine uses a fifth-order curve-fitting approximation originally
    // described by Jasper Vijn on coranac.com which has a worst-case
    // relative error of 0.07% (over [-pi:pi]).

    // Turn x from [0..2*PI] domain into [0..4] domain
    fixed x = *this % fixed::TAU;
    x = x / fixed::PI_DIV_2;

    // Take x modulo one rotation, so [-4..+4].
    if (x < fixed::from_int(0)) {
        x += fixed::from_int(4);
    }

    fixed sign = fixed::from_int(+1);
    if (x > fixed::from_int(2)) {
        // Reduce domain to [0..2].
        sign = fixed::from_int(-1);
        x -= fixed::from_int(2);
    }

    if (x > fixed::from_int(1)) {
        // Reduce domain to [0..1].
        x = fixed::from_int(2) - x;
    }

    const fixed x2 = x*x;
    return sign * x * (fixed::PI - x2*(fixed::TAU - fixed::from_int(5) - x2*(fixed::PI - fixed::from_int(3)))) >> 1;
}

fixed fixed::cos() const {
	return (*this + fixed::PI_DIV_2).sin();
}

fixed fixed::tan() const {
	fixed cx = cos();
	ERR_FAIL_COND_V_MSG(cx == fixed::ZERO, fixed::ZERO, "tan() of 90 degree angles is undefined");
	return sin() / cx;
}

fixed fixed::asin() const {
	if (value == 65536) {
		// libfixmath generates an incorrect result for asin(65536).
		// @todo Remove after we've replaced libfixmatch per issue #4:
		//       https://gitlab.com/snopek-games/sg-physics-2d/-/issues/4
		return fixed::PI_DIV_2;
	}
	if (value < fix16_maximum && value > fix16_minimum) {
		return fixed(fix16_asin(value));
	}

	int64_t remainder = value % fixed::TAU.value;
	return fixed(fix16_asin(remainder));
}

fixed fixed::acos() const {
	if (value == 65536) {
		// libfixmath generates an incorrect result for acos(65536).
		// @todo Remove after we've replaced libfixmatch per issue #4:
		//       https://gitlab.com/snopek-games/sg-physics-2d/-/issues/4
		return fixed::ZERO;
	}
	if (value < fix16_maximum && value > fix16_minimum) {
		return fixed(fix16_acos(value));
	}

	int64_t remainder = value % fixed::TAU.value;
	return fixed(fix16_acos(remainder));
}

fixed fixed::atan() const {
	if (value < fix16_maximum && value > fix16_minimum) {
		return fixed(fix16_atan(value));
	}

	int64_t remainder = value % fixed::TAU.value;
	return fixed(fix16_atan(remainder));
}

fixed fixed::atan2(const fixed &inY) const {
	if (value == 0 && inY.value == 0) {
		return fixed::ZERO;
	}
	if (value < fix16_maximum && value > fix16_minimum) {
		return fixed(fix16_atan2(value, inY.value));
	}

	int64_t x = value % fixed::PI.value;
	int64_t y = inY.value % fixed::PI.value;
	return fixed(fix16_atan2(x, y));
}

/* Copied from https://en.wikipedia.org/wiki/Exponentiation_by_squaring#With_constant_auxiliary_memory,
  * Copyrighted by Wikipedia editors and contributors
	* License: Creative Commons Attribution-ShareAlike 3.0 Unported License
	*/
fixed fixed::pow_integer(const fixed &exp) const {
	if (value < 0) {
		return fixed::ONE / (pow(exp * fixed::NEG_ONE));
	}

	fixed x = fixed(value);
	fixed y = fixed::ONE;
	fixed n = exp;

	if (n < fixed::ZERO) {
		x = fixed::ONE / x;
		n *= fixed::NEG_ONE;
	}

	if (n == fixed::ZERO) {
		return fixed::ONE;
	}

	while (n > fixed::ONE) {
		if ((n % fixed::TWO) == fixed::ZERO) {
			x *= x;
			n /= fixed::TWO;
		} else {
			y *= x;
			x *= x;
			n = (n - fixed::ONE) / fixed::TWO;
		}
	}
	return x * y;
}

/* Adapted from https://github.com/MikeLankamp/fpm/blob/master/include/fpm/math.hpp,
  * specifically the non-integer pow function at line 315. Calls the integer pow function if
	* the exponent is not a fraction.
	* Copyright 2019 Mike Lankamp
	* License: MIT
	*/
fixed fixed::pow(const fixed &exp) const {
	if (value == 0) {
		#ifdef MATH_CHECKS
			ERR_FAIL_COND_V_MSG(exp.value < 0, fixed::ARITHMETIC_OVERFLOW, "Cannot use negative exponents and a base of 0 with integer exponentiation!");
		#endif
		return fixed::ZERO;
	}

	if (exp < fixed::ZERO) {
		return fixed::ONE / (pow(exp * fixed::NEG_ONE));
	}

	if ((exp.value % 65536) == 0) {
		if (value < 0) {
			if (exp % fixed::TWO == fixed::ZERO) {
				return (fixed(value) * fixed::NEG_ONE).pow_integer(exp);
			} else {
				return (fixed(value) * fixed::NEG_ONE).pow_integer(exp)*fixed::NEG_ONE;
			}
		} else {
			return pow_integer(exp);
		}
	}

	#ifdef MATH_CHECKS
		ERR_FAIL_COND_V_MSG(base < fixed::ZERO, fixed::ARITHMETIC_OVERFLOW, "Negative bases and fractional exponents in pow() are not supported!");
	#endif
	return fixed(fix16_exp((fixed(fix16_log(value)) * exp).value));
}
