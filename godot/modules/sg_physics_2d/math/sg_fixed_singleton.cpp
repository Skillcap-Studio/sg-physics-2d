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

#include "sg_fixed_singleton.h"

#include "../internal/sg_fixed_number_internal.h"

SGFixed *SGFixed::singleton = NULL;

SGFixed::SGFixed() {
	ERR_FAIL_COND(singleton != NULL);
	singleton = this;
}

SGFixed::~SGFixed() {
	singleton = NULL;
}

SGFixed *SGFixed::get_singleton() {
	return singleton;
}

void SGFixed::_bind_methods() {
	ClassDB::bind_method(D_METHOD("from_int", "int_value"), &SGFixed::from_int);
	ClassDB::bind_method(D_METHOD("from_float", "float_value"), &SGFixed::from_float);
	ClassDB::bind_method(D_METHOD("to_int", "fixed_value"), &SGFixed::to_int);
	ClassDB::bind_method(D_METHOD("to_float", "fixed_value"), &SGFixed::to_float);
	ClassDB::bind_method(D_METHOD("mul", "fixed_one", "fixed_two"), &SGFixed::mul);
	ClassDB::bind_method(D_METHOD("div", "fixed_one", "fixed_two"), &SGFixed::div);

	ClassDB::bind_method(D_METHOD("sin", "fixed_value"), &SGFixed::sin);
	ClassDB::bind_method(D_METHOD("cos", "fixed_value"), &SGFixed::cos);
	ClassDB::bind_method(D_METHOD("tan", "fixed_value"), &SGFixed::tan);
	ClassDB::bind_method(D_METHOD("asin", "fixed_value"), &SGFixed::asin);
	ClassDB::bind_method(D_METHOD("acos", "fixed_value"), &SGFixed::acos);
	ClassDB::bind_method(D_METHOD("atan", "fixed_value"), &SGFixed::atan);
	ClassDB::bind_method(D_METHOD("atan2", "fixed_y_value", "fixed_x_value"), &SGFixed::atan2);
	ClassDB::bind_method(D_METHOD("sqrt", "fixed_value"), &SGFixed::sqrt);

	ClassDB::bind_method(D_METHOD("vector2", "fixed_x", "fixed_y"), &SGFixed::vector2);
	ClassDB::bind_method(D_METHOD("from_float_vector2", "float_vector"), &SGFixed::from_float_vector2);

	ClassDB::bind_method(D_METHOD("rect2", "position", "size"), &SGFixed::rect2);
	ClassDB::bind_method(D_METHOD("from_float_rect2", "float_rect"), &SGFixed::from_float_rect2);

	ClassDB::bind_method(D_METHOD("transform2d", "rotation", "origin"), &SGFixed::transform2d);

	BIND_CONSTANT(ONE);
	BIND_CONSTANT(HALF);
	BIND_CONSTANT(TWO);
	BIND_CONSTANT(NEG_ONE);
	BIND_CONSTANT(PI);
	BIND_CONSTANT(TAU);
	BIND_CONSTANT(PI_DIV_2);
	BIND_CONSTANT(PI_DIV_4);
	BIND_CONSTANT(EPSILON);

}

int64_t SGFixed::from_int(int64_t p_int_value) const {
	return fixed::from_int(p_int_value).value;
}

int64_t SGFixed::from_float(float p_float_value) const {
	return fixed::from_float(p_float_value).value;
}

int64_t SGFixed::to_int(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).to_int();
}

float SGFixed::to_float(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).to_float();
}

int64_t SGFixed::mul(int64_t p_fixed_one, int64_t p_fixed_two) const {
	return (fixed(p_fixed_one) * fixed(p_fixed_two)).value;
}

int64_t SGFixed::div(int64_t p_fixed_one, int64_t p_fixed_two) const {
	return (fixed(p_fixed_one) / fixed(p_fixed_two)).value;
}

int64_t SGFixed::sin(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).sin().value;
}

int64_t SGFixed::cos(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).cos().value;
}

int64_t SGFixed::tan(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).tan().value;
}

int64_t SGFixed::asin(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).asin().value;
}

int64_t SGFixed::acos(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).acos().value;
}

int64_t SGFixed::atan(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).atan().value;
}

int64_t SGFixed::atan2(int64_t p_fixed_y_value, int64_t p_fixed_x_value) const {
	return fixed(p_fixed_y_value).atan2(fixed(p_fixed_x_value)).value;
}

int64_t SGFixed::sqrt(int64_t p_fixed_value) const {
	return fixed(p_fixed_value).sqrt().value;
}

Ref<SGFixedVector2> SGFixed::vector2(int64_t p_fixed_x, int64_t p_fixed_y) const {
	return Ref<SGFixedVector2>(memnew(SGFixedVector2(SGFixedVector2Internal(fixed(p_fixed_x), fixed(p_fixed_y)))));
}

Ref<SGFixedVector2> SGFixed::from_float_vector2(const Vector2 &p_float_vector) const {
	return Ref<SGFixedVector2>(memnew(SGFixedVector2(SGFixedVector2Internal(fixed::from_float(p_float_vector.x), fixed::from_float(p_float_vector.y)))));
}

Ref<SGFixedRect2> SGFixed::rect2(const Ref<SGFixedVector2> &p_position, const Ref<SGFixedVector2> &p_size) const {
	Ref<SGFixedRect2> ret(memnew(SGFixedRect2));
	ret->set_position(p_position);
	ret->set_size(p_size);
	return ret;
}

Ref<SGFixedRect2> SGFixed::from_float_rect2(const Rect2 &p_float_rect) const {
	Ref<SGFixedRect2> ret(memnew(SGFixedRect2));
	ret->set_internal(SGFixedRect2Internal::from_float(p_float_rect));
	return ret;
}

Ref<SGFixedTransform2D> SGFixed::transform2d(int64_t p_rotation, const Ref<SGFixedVector2> &p_origin) const {
	Ref<SGFixedTransform2D> ret(memnew(SGFixedTransform2D));
	ret->set_internal(SGFixedTransform2DInternal(fixed(p_rotation), p_origin->get_internal()));
	return ret;
}
