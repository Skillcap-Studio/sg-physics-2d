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

// Code originally from Godot Engine's Curve (MIT License)

#ifndef SG_CURVE_H
#define SG_CURVE_H

#include "core/resource.h"
#include "../../math/sg_fixed_vector2.h"

class SGCurve2D : public Resource {
	GDCLASS(SGCurve2D, Resource);

	struct Point {
		SGFixedVector2Internal in;
		SGFixedVector2Internal out;
		SGFixedVector2Internal pos;
	};

	Vector<Point> points;

	struct BakedPoint {
		fixed ofs;
		SGFixedVector2Internal point;
	};

	mutable bool baked_cache_dirty;
	mutable Vector<SGFixedVector2Internal> baked_point_cache;
	mutable fixed baked_max_ofs;

	void _bake() const;

	fixed bake_interval;

	void _bake_segment2d(Map<fixed, SGFixedVector2Internal>& r_bake, fixed p_begin, fixed p_end, const SGFixedVector2Internal& p_a, const SGFixedVector2Internal& p_out, const SGFixedVector2Internal& p_b, const SGFixedVector2Internal& p_in, int p_depth, int p_max_depth, fixed p_tol) const;
	Dictionary _get_data() const;
	void _set_data(const Dictionary& p_data);

protected:
	friend class SGPath2D;
	friend class SGPathFollow2D;

	static void _bind_methods();

	SGFixedVector2Internal get_point_position_internal(int p_index) const;
	void set_bake_interval_internal(fixed p_tolerance);
	SGFixedVector2Internal interpolate_internal(int p_index, fixed p_offset) const;
	SGFixedVector2Internal interpolatef_internal(fixed p_findex) const;
	SGFixedVector2Internal interpolate_baked_internal(fixed p_offset, bool p_cubic) const;
	Vector<SGFixedVector2Internal> get_baked_points_internal() const;
	SGFixedVector2Internal get_closest_point_internal(const SGFixedVector2Internal& p_to_point) const;
	fixed get_closest_offset_internal(const SGFixedVector2Internal& p_to_point) const;
	Vector<SGFixedVector2Internal> tessellate_internal(int p_max_stages, fixed p_tolerance) const;

public:
	int get_point_count() const;
	void add_point(Ref<SGFixedVector2> p_pos, Ref<SGFixedVector2> p_in = nullptr, Ref<SGFixedVector2> p_out = nullptr, int p_atpos = -1);
	void set_point_position(int p_index, Ref<SGFixedVector2> p_pos);
	Ref<SGFixedVector2> get_point_position(int p_index) const;
	void set_point_in(int p_index, Ref<SGFixedVector2> p_in);
	Ref<SGFixedVector2> get_point_in(int p_index) const;
	void set_point_out(int p_index, Ref<SGFixedVector2> p_out);
	Ref<SGFixedVector2> get_point_out(int p_index) const;
	void remove_point(int p_index);
	void clear_points();

	Ref<SGFixedVector2> interpolate(int p_index, int64_t p_offset) const;
	Ref<SGFixedVector2> interpolatef(int64_t p_findex) const;

	void set_bake_interval(int64_t p_tolerance);
	int64_t get_bake_interval() const;

	int64_t get_baked_length() const;
	Ref<SGFixedVector2> interpolate_baked(int64_t p_offset, bool p_cubic = false) const;
	Array get_baked_points() const; //useful for going through
	Ref<SGFixedVector2> get_closest_point(const Ref<SGFixedVector2> p_to_point) const;
	int64_t get_closest_offset(const Ref<SGFixedVector2> p_to_point) const;

	Array tessellate(int p_max_stages = 5, int64_t p_tolerance = 262144) const; //useful for display

	SGCurve2D();
};

#endif // SG_CURVE_H
