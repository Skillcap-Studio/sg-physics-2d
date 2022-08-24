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

#ifndef SG_FIXED_NUMBER_INTERNAL_H
#define SG_FIXED_NUMBER_INTERNAL_H

#include <core/typedefs.h>
#include <core/error_macros.h>
#include <core/ustring.h>
#include "core/vector.h"


int64_t sg_sqrt_64(int64_t num);

// To avoid overflow related to flipping the sign on twos-complement
// representation of of the minimum integer, we effectively limit values to
// 1 above the minimum.
#define INT64_EFFECTIVE_MIN (INT64_MIN + 1)

#ifdef DEBUG_ENABLED
#define SG_FIXED_MATH_CHECKS
#endif

#define FIXED_SGN(m_v) (((m_v) < fixed::ZERO) ? fixed::NEG_ONE : fixed::ONE)

struct fixed {
	int64_t value;

	_FORCE_INLINE_ fixed() : value(0) {}

	explicit _FORCE_INLINE_ fixed(int64_t p_initial_value)
		: value(p_initial_value) {}

	static const fixed ZERO;
	static const fixed ONE;
	static const fixed HALF;
	static const fixed TWO;
	static const fixed NEG_ONE;
	static const fixed PI;
	static const fixed TAU;
	static const fixed PI_DIV_2;
	static const fixed PI_DIV_4;
	static const fixed EPSILON;
	static const fixed ARITHMETIC_OVERFLOW;
	static const fixed DEG_180;
	static const int64_t TENS[];
	static const int TENS_SIZE;

	static _FORCE_INLINE_ fixed from_int(int64_t p_int_value) {
		return fixed(p_int_value << 16);
	}

	static _FORCE_INLINE_ fixed from_float(float p_float_value) {
		return fixed(p_float_value * 65536);
	}

	static fixed from_string(String p_string_value) {
		if (!(p_string_value.is_valid_float() || p_string_value.is_valid_integer())){
			return fixed::ZERO;
		}

		int64_t value = 0;
		Vector<String> str_values = p_string_value.split(".", false);
		if ( !str_values.empty()) {
			value = str_values.get(0).to_int64() << 16;
			if (str_values.size() > 1){
				if(str_values.get(1).length() >= TENS_SIZE)
					str_values.set(1, str_values.get(1).substr(0, TENS_SIZE-1));

				value += (str_values.get(1).to_int64() << 16) / TENS[str_values.get(1).length()];
			}
		}
		return fixed(value);
	}

	static _FORCE_INLINE_ bool is_equal_approx(fixed a, fixed b) {
		if (a == b) {
			return true;
		}
		fixed tolerance = fixed::EPSILON * a.abs();
		if (tolerance < fixed::EPSILON) {
			tolerance = fixed::EPSILON;
		}
		return (a - b).abs() < tolerance;
	}

	static _FORCE_INLINE_ bool is_equal_approx(fixed a, fixed b, fixed tolerance) {
		if (a == b) {
			return true;
		}
		return (a - b).abs() < tolerance;
	}

	_FORCE_INLINE_ fixed deg2rad(){
		return *this * fixed::PI / fixed::DEG_180;
	}

	_FORCE_INLINE_ fixed rad2deg(){
		return *this * fixed::DEG_180 / fixed::PI;
	}

	_FORCE_INLINE_ fixed floor() {
		return fixed(value & ~0xFFFF);
	}

	_FORCE_INLINE_ fixed round() {
		return (*this + fixed::HALF).floor();
	}

	_FORCE_INLINE_ fixed ceil() {
		return (value & 0xFFFF)? (*this + fixed::ONE).floor() : *this;
	}

	_FORCE_INLINE_ int64_t to_int() const {
		return value >> 16;
	}

	_FORCE_INLINE_ float to_float() const {
		return (float)value / 65536.0f;
	}

	String format_string() const {
		String str = String::num_int64(value >> 16);
		int64_t decimal = value & 0xFFFF;
		if(decimal != 0) {
			decimal = (decimal * 10000000) >> 16;  //10.000.000 - for precision

			//add left zeros
			String left_zeros = "";
			int64_t left_zero_checker = 1000000;
			while(left_zero_checker > decimal) {
				left_zero_checker /= 10;
				left_zeros += "0";
			}

			//right 0s, as decimals, are useless info: remove them
			while (decimal % 10 == 0)
				decimal /= 10;

			str += "." + left_zeros + String::num_int64(decimal);
		}
		return str;
	}

	_FORCE_INLINE_ fixed operator+(const fixed& p_other) const {
		#ifdef SG_FIXED_MATH_CHECKS
		ERR_FAIL_COND_V_MSG(p_other.value > 0 && (value > INT64_MAX - p_other.value),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point addition overflow");
		ERR_FAIL_COND_V_MSG(p_other.value < 0 && (value < INT64_EFFECTIVE_MIN - p_other.value),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point addition overflow");
		#endif

		return fixed(value + p_other.value);
	}

	_FORCE_INLINE_ fixed& operator+=(const fixed& p_other) {
		value = (*this + p_other).value;
		return *this;
	}

	_FORCE_INLINE_ fixed operator-(const fixed& p_other) const {
		#ifdef SG_FIXED_MATH_CHECKS
		ERR_FAIL_COND_V_MSG(p_other.value < 0 && (value > INT64_MAX + p_other.value),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point subtraction overflow");
		ERR_FAIL_COND_V_MSG(p_other.value > 0 && (value < INT64_EFFECTIVE_MIN + p_other.value),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point subtraction overflow");
		#endif

		return fixed(value - p_other.value);
	}

	_FORCE_INLINE_ fixed& operator-=(const fixed& p_other) {
		value = (*this - p_other).value;
		return *this;
	}

	_FORCE_INLINE_ fixed operator*(const fixed& p_other) const {
		#ifdef SG_FIXED_MATH_CHECKS
		ERR_FAIL_COND_V_MSG(value == -1 && p_other.value == INT64_MAX,
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point multiplication overflow");
		ERR_FAIL_COND_V_MSG(p_other.value == -1 && value == INT64_MIN,
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point multiplication overflow");
		ERR_FAIL_COND_V_MSG(p_other.value > 0 && (value > (INT64_MAX / p_other.value) || value < (INT64_EFFECTIVE_MIN / p_other.value)),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point multiplication overflow");
		ERR_FAIL_COND_V_MSG(p_other.value < 0 && (value < (INT64_MAX / p_other.value) || value > (INT64_EFFECTIVE_MIN / p_other.value)),
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point multiplication overflow");
		#endif

		return fixed((value * p_other.value) >> 16);
	}

	_FORCE_INLINE_ fixed& operator*=(const fixed& p_other) {
		value = (*this * p_other).value;
		return *this;
	}

	_FORCE_INLINE_ fixed operator/(const fixed& p_other) const {
		#ifdef SG_FIXED_MATH_CHECKS
		ERR_FAIL_COND_V_MSG(value == -1 && p_other.value == INT64_MAX,
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point division overflow");
		ERR_FAIL_COND_V_MSG(p_other.value == -1 && value == INT64_MIN,
			fixed::ARITHMETIC_OVERFLOW, "Fixed-point division overflow");
		#endif

		return fixed((value << 16) / p_other.value);
	}

	_FORCE_INLINE_ fixed& operator/=(const fixed& p_other) {
		value = (*this / p_other).value;
		return *this;
	}

	_FORCE_INLINE_ fixed operator<<(size_t pos) const {
		return fixed(value << pos);
	}
	_FORCE_INLINE_ fixed operator>>(size_t pos) const {
		return fixed(value >> pos);
	}

	_FORCE_INLINE_ fixed operator%(fixed const& rhs) const {
		return fixed(value % rhs.value);
	}

	_FORCE_INLINE_ bool operator==(const fixed &p_other) const { return value == p_other.value; }
	_FORCE_INLINE_ bool operator!=(const fixed &p_other) const { return value != p_other.value; }
	_FORCE_INLINE_ bool operator<=(const fixed &p_other) const { return value <= p_other.value; }
	_FORCE_INLINE_ bool operator>=(const fixed &p_other) const { return value >= p_other.value; }
	_FORCE_INLINE_ bool operator< (const fixed &p_other) const { return value <  p_other.value; }
	_FORCE_INLINE_ bool operator> (const fixed &p_other) const { return value >  p_other.value; }

	_FORCE_INLINE_ fixed abs() const { return (value < 0) ? fixed(-value) : *this; }
	_FORCE_INLINE_ fixed operator-() const { return fixed(-value); }
	_FORCE_INLINE_ fixed sqrt() const { return fixed(sg_sqrt_64(value << 16)); }
	_FORCE_INLINE_ fixed sign() const { return value < 0 ? fixed::NEG_ONE : (value > 0 ? fixed::ONE : fixed::ZERO); }
	_FORCE_INLINE_ fixed move_toward(const fixed& p_other, fixed p_delta) { return (p_other - *this).abs() <= p_delta ? p_other : *this + FIXED_SGN(p_other - *this) * p_delta; }

	fixed pow(const fixed &exp) const;
	fixed pow_integer(const fixed &exp) const;
	fixed  sin() const;
	fixed  cos() const;
	fixed  tan() const;
	fixed asin() const;
	fixed acos() const;
	fixed atan() const;
	fixed atan2(const fixed &inX) const;

	// Calculates atan(x), assuming that x is in the range 0 to 1.
	static fixed atan_sanitized(const fixed &p_x);
	// Calculates atan(y / x), assuming x != 0.
	static fixed atan_div(const fixed &p_y, const fixed &p_x);
};

#endif
