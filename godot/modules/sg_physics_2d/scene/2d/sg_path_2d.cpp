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

// Code originally from Godot Engine's Path2D (MIT License)

#include "sg_path_2d.h"

#include "core/engine.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_scale.h"
#endif

#ifdef TOOLS_ENABLED
Rect2 SGPath2D::_edit_get_rect() const {
	if (!curve.is_valid() || curve->get_point_count() == 0) {
		return Rect2(0, 0, 0, 0);
	}

	Rect2 aabb = Rect2(curve->get_point_position(0)->to_float(), Vector2(0, 0));

	for (int i = 0; i < curve->get_point_count(); i++) {
		for (int j = 0; j <= 8; j++) {
			real_t frac = j / 8.0;
			Vector2 p = curve->interpolate(i, fixed::from_float(frac).value)->to_float();
			aabb.expand_to(p);
		}
	}

	return aabb;
}

bool SGPath2D::_edit_use_rect() const {
	return curve.is_valid() && curve->get_point_count() != 0;
}

bool SGPath2D::_edit_is_selected_on_click(const Point2& p_point, double p_tolerance) const {
	if (curve.is_null()) {
		return false;
	}

	for (int i = 0; i < curve->get_point_count(); i++) {
		Vector2 s[2];
		s[0] = curve->get_point_position(i)->to_float();

		for (int j = 1; j <= 8; j++) {
			real_t frac = j / 8.0;
			s[1] = curve->interpolate(i, fixed::from_float(frac).value)->to_float();

			Vector2 p = Geometry::get_closest_point_to_segment_2d(p_point, s);
			if (p.distance_to(p_point) <= p_tolerance) {
				return true;
			}

			s[0] = s[1];
		}
	}

	return false;
}
#endif

void SGPath2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_DRAW && curve.is_valid()) {
		//draw the curve!!

		if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_navigation_hint()) {
			return;
		}

#ifdef TOOLS_ENABLED
		const float line_width = 2 * EDSCALE;
#else
		const float line_width = 2;
#endif
		const Color color = Color(1.0, 1.0, 1.0, 1.0);

		for (int i = 0; i < curve->get_point_count(); i++) {
			Vector2 prev_p = curve->get_point_position(i)->to_float();

			for (int j = 1; j <= 8; j++) {
				fixed frac = fixed::from_float(j / 8.0);
				Vector2 p = curve->interpolate(i, frac.value)->to_float();
				draw_line(prev_p, p, color, line_width, true);
				prev_p = p;
			}
		}
	}
}

void SGPath2D::_curve_changed() {
	if (!is_inside_tree()) {
		return;
	}

	if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_navigation_hint()) {
		return;
	}

	update();
}

void SGPath2D::set_curve(const Ref<SGCurve2D>& p_curve) {
	if (curve.is_valid()) {
		curve->disconnect("changed", this, "_curve_changed");
	}

	curve = p_curve;

	if (curve.is_valid()) {
		curve->connect("changed", this, "_curve_changed");
	}

	_curve_changed();
}

Ref<SGCurve2D> SGPath2D::get_curve() const {
	return curve;
}

void SGPath2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &SGPath2D::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &SGPath2D::get_curve);
	ClassDB::bind_method(D_METHOD("_curve_changed"), &SGPath2D::_curve_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_RESOURCE_TYPE, "SGCurve2D"), "set_curve", "get_curve");
}

SGPath2D::SGPath2D() {
	set_curve(Ref<SGCurve2D>(memnew(SGCurve2D))); //create one by default
	set_self_modulate(Color(0.5, 0.6, 1.0, 0.7));
}

/////////////////////////////////////////////////////////////////////////////////

void SGPathFollow2D::_update_transform() {
	if (!path) {
		return;
	}

	Ref<SGCurve2D> c = path->get_curve();
	if (!c.is_valid()) {
		return;
	}

	fixed path_length = fixed(c->get_baked_length());
	if (path_length == fixed(0)) {
		return;
	}
	SGFixedVector2Internal pos = c->interpolate_baked_internal(offset, cubic);

	if (rotate) {
		fixed ahead = offset + lookahead;

		if (loop && ahead >= path_length) {
			// If our lookahead will loop, we need to check if the path is closed.
			int point_count = c->get_point_count();
			if (point_count > 0) {
				SGFixedVector2Internal start_point = c->get_point_position_internal(0);
				SGFixedVector2Internal end_point = c->get_point_position_internal(point_count - 1);
				if (start_point == end_point) {
					// Since the path is closed we want to 'smooth off'
					// the corner at the start/end.
					// So we wrap the lookahead back round.
					ahead = ahead % path_length;
				}
			}
		}

		SGFixedVector2Internal ahead_pos = c->interpolate_baked_internal(ahead, cubic);

		SGFixedVector2Internal tangent_to_curve;
		if (ahead_pos == pos) {
			// This will happen at the end of non-looping or non-closed paths.
			// We'll try a look behind instead, in order to get a meaningful angle.
			tangent_to_curve =
				(pos - c->interpolate_baked_internal(offset - lookahead, cubic)).normalized();
		}
		else {
			tangent_to_curve = (ahead_pos - pos).normalized();
		}

		SGFixedVector2Internal normal_of_curve = -tangent_to_curve.tangent();

		pos += tangent_to_curve * h_offset;
		pos += normal_of_curve * v_offset;

		set_fixed_rotation(tangent_to_curve.angle().value);

	}
	else {
		pos.x += h_offset;
		pos.y += v_offset;
	}

	set_fixed_position_internal(pos);
}

void SGPathFollow2D::_notification(int p_what) {
	switch (p_what) {
	case NOTIFICATION_ENTER_TREE: {
		path = Object::cast_to<SGPath2D>(get_parent());
		if (path) {
			_update_transform();
		}

	} break;
	case NOTIFICATION_EXIT_TREE: {
		path = nullptr;
	} break;
	}
}

void SGPathFollow2D::set_cubic_interpolation(bool p_enable) {
	cubic = p_enable;
}

bool SGPathFollow2D::get_cubic_interpolation() const {
	return cubic;
}

void SGPathFollow2D::_validate_property(PropertyInfo& property) const {
	if (property.name == "offset") {
		float max = 10000;
		if (path && path->get_curve().is_valid()) {
			max = fixed(path->get_curve()->get_baked_length()).to_float();
		}

		property.hint_string = "0," + rtos(max) + ",0.01,or_lesser,or_greater";
	}
}

String SGPathFollow2D::get_configuration_warning() const {
	if (!is_visible_in_tree() || !is_inside_tree()) {
		return String();
	}

	String warning = Node2D::get_configuration_warning();
	if (!Object::cast_to<SGPath2D>(get_parent())) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("SGPathFollow2D only works when set as a child of a SGPath2D node.");
	}

	return warning;
}

void SGPathFollow2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &SGPathFollow2D::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &SGPathFollow2D::get_offset);

	ClassDB::bind_method(D_METHOD("set_h_offset", "h_offset"), &SGPathFollow2D::set_h_offset);
	ClassDB::bind_method(D_METHOD("get_h_offset"), &SGPathFollow2D::get_h_offset);

	ClassDB::bind_method(D_METHOD("set_v_offset", "v_offset"), &SGPathFollow2D::set_v_offset);
	ClassDB::bind_method(D_METHOD("get_v_offset"), &SGPathFollow2D::get_v_offset);

	ClassDB::bind_method(D_METHOD("set_unit_offset", "unit_offset"), &SGPathFollow2D::set_unit_offset);
	ClassDB::bind_method(D_METHOD("get_unit_offset"), &SGPathFollow2D::get_unit_offset);

	ClassDB::bind_method(D_METHOD("set_rotate", "enable"), &SGPathFollow2D::set_rotate);
	ClassDB::bind_method(D_METHOD("is_rotating"), &SGPathFollow2D::is_rotating);

	ClassDB::bind_method(D_METHOD("set_cubic_interpolation", "enable"), &SGPathFollow2D::set_cubic_interpolation);
	ClassDB::bind_method(D_METHOD("get_cubic_interpolation"), &SGPathFollow2D::get_cubic_interpolation);

	ClassDB::bind_method(D_METHOD("set_loop", "loop"), &SGPathFollow2D::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &SGPathFollow2D::has_loop);

	ClassDB::bind_method(D_METHOD("set_lookahead", "lookahead"), &SGPathFollow2D::set_lookahead);
	ClassDB::bind_method(D_METHOD("get_lookahead"), &SGPathFollow2D::get_lookahead);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "offset", PROPERTY_HINT_RANGE, "0,10000,0.01,or_lesser,or_greater"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "unit_offset", PROPERTY_HINT_RANGE, "0,1,0.0001,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_unit_offset", "get_unit_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "h_offset"), "set_h_offset", "get_h_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "v_offset"), "set_v_offset", "get_v_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rotate"), "set_rotate", "is_rotating");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cubic_interp"), "set_cubic_interpolation", "get_cubic_interpolation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lookahead", PROPERTY_HINT_RANGE, "0.001,1024.0,0.001"), "set_lookahead", "get_lookahead");
}

void SGPathFollow2D::set_offset(int64_t p_offset) {
	offset = fixed(p_offset);
	if (path) {
		if (path->get_curve().is_valid()) {
			fixed path_length(path->get_curve()->get_baked_length());

			if (loop) {
				offset = offset % path_length;
				if (p_offset != 0 && offset == fixed(0)) {
					offset = path_length;
				}
			}
			else {
				offset = CLAMP(offset, fixed(0), path_length);
			}
		}

		_update_transform();
	}
	_change_notify("offset");
	_change_notify("unit_offset");
}

void SGPathFollow2D::set_h_offset(int64_t p_h_offset) {
	h_offset = fixed(p_h_offset);
	if (path) {
		_update_transform();
	}
}

int64_t SGPathFollow2D::get_h_offset() const {
	return h_offset.value;
}

void SGPathFollow2D::set_v_offset(int64_t p_v_offset) {
	v_offset = fixed(p_v_offset);
	if (path) {
		_update_transform();
	}
}

int64_t SGPathFollow2D::get_v_offset() const {
	return v_offset.value;
}

int64_t SGPathFollow2D::get_offset() const {
	return offset.value;
}

void SGPathFollow2D::set_unit_offset(int64_t p_unit_offset) {
	if (path && path->get_curve().is_valid() && path->get_curve()->get_baked_length()) {
		set_offset((fixed(p_unit_offset) * fixed(path->get_curve()->get_baked_length())).value);
	}
}

int64_t SGPathFollow2D::get_unit_offset() const {
	if (path && path->get_curve().is_valid() && path->get_curve()->get_baked_length()) {
		return (fixed(get_offset()) / fixed(path->get_curve()->get_baked_length())).value;
	}
	else {
		return 0;
	}
}

void SGPathFollow2D::set_lookahead(int64_t p_lookahead) {
	lookahead = fixed(p_lookahead);
}

int64_t SGPathFollow2D::get_lookahead() const {
	return lookahead.value;
}

void SGPathFollow2D::set_rotate(bool p_rotate) {
	rotate = p_rotate;
	_update_transform();
}

bool SGPathFollow2D::is_rotating() const {
	return rotate;
}

void SGPathFollow2D::set_loop(bool p_loop) {
	loop = p_loop;
}

bool SGPathFollow2D::has_loop() const {
	return loop;
}

SGPathFollow2D::SGPathFollow2D() {
	offset = fixed(0);
	h_offset = fixed(0);
	v_offset = fixed(0);
	path = nullptr;
	rotate = true;
	cubic = true;
	loop = true;
	lookahead = fixed(262144);
}
