/*************************************************************************/
/* Copyright (c) 2021 David Snopek                                       */
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

#include "sg_fixed_node_2d.h"

#include <core/engine.h>

void SGFixedNode2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_fixed_transform"), &SGFixedNode2D::get_fixed_transform);
	ClassDB::bind_method(D_METHOD("set_fixed_transform", "fixed_transform"), &SGFixedNode2D::set_fixed_transform);
	ClassDB::bind_method(D_METHOD("_fixed_transform_changed"), &SGFixedNode2D::_fixed_transform_changed);

	ClassDB::bind_method(D_METHOD("get_fixed_position"), &SGFixedNode2D::get_fixed_position);
	ClassDB::bind_method(D_METHOD("set_fixed_position", "fixed_position"), &SGFixedNode2D::set_fixed_position);
	ClassDB::bind_method(D_METHOD("_fixed_position_changed"), &SGFixedNode2D::_fixed_position_changed);

	ClassDB::bind_method(D_METHOD("get_fixed_scale"), &SGFixedNode2D::get_fixed_scale);
	ClassDB::bind_method(D_METHOD("set_fixed_scale", "fixed_scale"), &SGFixedNode2D::set_fixed_scale);
	ClassDB::bind_method(D_METHOD("_fixed_scale_changed"), &SGFixedNode2D::_fixed_scale_changed);

	ClassDB::bind_method(D_METHOD("get_fixed_rotation"), &SGFixedNode2D::get_fixed_rotation);
	ClassDB::bind_method(D_METHOD("set_fixed_rotation", "fixed_scale"), &SGFixedNode2D::set_fixed_rotation);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fixed_transform", PROPERTY_HINT_NONE, "", 0), "set_fixed_transform", "get_fixed_transform");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fixed_position", PROPERTY_HINT_TYPE_STRING, "SGFixedVector2", PROPERTY_USAGE_EDITOR), "set_fixed_position", "get_fixed_position");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fixed_scale", PROPERTY_HINT_TYPE_STRING, "SGFixedVector2", PROPERTY_USAGE_EDITOR), "set_fixed_scale", "get_fixed_scale");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_rotation"), "set_fixed_rotation", "get_fixed_rotation");

	ClassDB::bind_method(D_METHOD("get_global_fixed_transform"), &SGFixedNode2D::get_global_fixed_transform);
	ClassDB::bind_method(D_METHOD("set_global_fixed_transform", "fixed_transform"), &SGFixedNode2D::set_global_fixed_transform);

	ClassDB::bind_method(D_METHOD("get_global_fixed_position"), &SGFixedNode2D::get_global_fixed_position);
	ClassDB::bind_method(D_METHOD("set_global_fixed_position", "fixed_position"), &SGFixedNode2D::set_global_fixed_position);

	ClassDB::bind_method(D_METHOD("get_global_fixed_rotation"), &SGFixedNode2D::get_global_fixed_rotation);
	ClassDB::bind_method(D_METHOD("set_global_fixed_rotation", "fixed_rotation"), &SGFixedNode2D::set_global_fixed_rotation);

	//
	// For storage in TSCN and SCN files only.
	//

	ClassDB::bind_method(D_METHOD("_get_fixed_position_x"), &SGFixedNode2D::_get_fixed_position_x);
	ClassDB::bind_method(D_METHOD("_set_fixed_position_x", "x"), &SGFixedNode2D::_set_fixed_position_x);
	ClassDB::bind_method(D_METHOD("_get_fixed_position_y"), &SGFixedNode2D::_get_fixed_position_y);
	ClassDB::bind_method(D_METHOD("_set_fixed_position_y", "y"), &SGFixedNode2D::_set_fixed_position_y);
	ClassDB::bind_method(D_METHOD("_get_fixed_scale_x"), &SGFixedNode2D::_get_fixed_scale_x);
	ClassDB::bind_method(D_METHOD("_set_fixed_scale_x", "x"), &SGFixedNode2D::_set_fixed_scale_x);
	ClassDB::bind_method(D_METHOD("_get_fixed_scale_y"), &SGFixedNode2D::_get_fixed_scale_y);
	ClassDB::bind_method(D_METHOD("_set_fixed_scale_y", "y"), &SGFixedNode2D::_set_fixed_scale_y);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_position_x", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_fixed_position_x", "_get_fixed_position_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_position_y", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_fixed_position_y", "_get_fixed_position_y");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_scale_x", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_fixed_scale_x", "_get_fixed_scale_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_scale_y", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_fixed_scale_y", "_get_fixed_scale_y");

}

void SGFixedNode2D::_changed_callback(Object *p_changed, const char *p_prop) {
	if (!updating_transform) {
		if (strcmp(p_prop, "position") == 0) {
			fixed_position->from_float(get_position());
			set_fixed_position(fixed_position);
		}
		else if (strcmp(p_prop, "scale") == 0) {
			fixed_scale->from_float(get_scale());
			set_fixed_scale(fixed_scale);
		}
		else if (strcmp(p_prop, "rotation") == 0) {
			fixed_rotation = fixed::from_float(get_rotation()).value;
			set_fixed_rotation(fixed_rotation);
		}
		else if (strcmp(p_prop, "transform") == 0) {
			//fixed_transform->from_float(get_transform());
			//set_fixed_transform(fixed_transform);
		}
	}
}

SGFixedTransform2DInternal SGFixedNode2D::get_global_fixed_transform_internal() const {
	SGFixedNode2D *fixed_parent = Object::cast_to<SGFixedNode2D>(get_parent());
	if (fixed_parent) {
		return fixed_parent->get_global_fixed_transform_internal() * fixed_transform->get_internal();
	}
	return fixed_transform->get_internal();
}

void SGFixedNode2D::update_fixed_transform_internal(const SGFixedTransform2DInternal &p_transform) {
	updating_transform = true;
	fixed_transform->set_internal(p_transform);
	fixed_position->set_internal(p_transform.get_origin());
	fixed_scale->set_internal(p_transform.get_scale());
	fixed_rotation = p_transform.get_rotation().value;
	set_transform(fixed_transform->to_float());
	_change_notify("fixed_position");
	_change_notify("fixed_scale");
	_change_notify("fixed_rotation");
	updating_transform = false;
}

void SGFixedNode2D::update_global_fixed_transform_internal(const SGFixedTransform2DInternal &p_global_transform) {
	SGFixedNode2D *fixed_parent = Object::cast_to<SGFixedNode2D>(get_parent());
	if (fixed_parent) {
		update_fixed_transform_internal(fixed_parent->get_global_fixed_transform_internal().affine_inverse() * p_global_transform);
	}
	else {
		update_fixed_transform_internal(p_global_transform);
	}
}

void SGFixedNode2D::_set_fixed_position(const SGFixedVector2Internal &p_fixed_position) {
	fixed_position->set_internal(p_fixed_position);
	updating_transform = true;
	set_position(fixed_position->to_float());
	fixed_transform->set_origin(fixed_position);
	updating_transform = false;
	_change_notify("fixed_position");
}

void SGFixedNode2D::_fixed_transform_changed() {
	if (!updating_transform) {
		set_fixed_transform(fixed_transform);
	}
}

void SGFixedNode2D::_fixed_position_changed() {
	if (!updating_transform) {
		set_fixed_position(fixed_position);
	}
}

void SGFixedNode2D::_fixed_scale_changed() {
	if (!updating_transform) {
		set_fixed_scale(fixed_scale);
	}
}

int64_t SGFixedNode2D::_get_fixed_position_x() const {
	return fixed_position->get_x();
}

void SGFixedNode2D::_set_fixed_position_x(int64_t p_x) {
	fixed_position->set_x(p_x);
}

int64_t SGFixedNode2D::_get_fixed_position_y() const {
	return fixed_position->get_y();
}

void SGFixedNode2D::_set_fixed_position_y(int64_t p_y) {
	fixed_position->set_y(p_y);
}

int64_t SGFixedNode2D::_get_fixed_scale_x() const {
	return fixed_scale->get_x();
}

void SGFixedNode2D::_set_fixed_scale_x(int64_t p_x) {
	fixed_scale->set_x(p_x);
}

int64_t SGFixedNode2D::_get_fixed_scale_y() const {
	return fixed_scale->get_y();
}

void SGFixedNode2D::_set_fixed_scale_y(int64_t p_y) {
	fixed_scale->set_y(p_y);
}

void SGFixedNode2D::set_fixed_transform(const Ref<SGFixedTransform2D> &p_fixed_transform) {
	ERR_FAIL_COND(!p_fixed_transform.is_valid());
	update_fixed_transform_internal(p_fixed_transform->get_internal());
}

Ref<SGFixedTransform2D> SGFixedNode2D::get_fixed_transform() const {
	return fixed_transform;
}

void SGFixedNode2D::set_fixed_position(const Ref<SGFixedVector2> &p_fixed_position) {
	ERR_FAIL_COND(!p_fixed_position.is_valid());
	_set_fixed_position(p_fixed_position->get_internal());
}

Ref<SGFixedVector2> SGFixedNode2D::get_fixed_position() {
	return fixed_position;
}

void SGFixedNode2D::set_fixed_scale(const Ref<SGFixedVector2> &p_fixed_scale) {
	ERR_FAIL_COND(!p_fixed_scale.is_valid());

	fixed_scale->set_internal(p_fixed_scale->get_internal());
	updating_transform = true;
	set_scale(fixed_scale->to_float());

	SGFixedTransform2DInternal internal_transform = fixed_transform->get_internal();
	internal_transform.set_scale(fixed_scale->get_internal());
	fixed_transform->set_internal(internal_transform);

	updating_transform = false;
	_change_notify("fixed_scale");
}

Ref<SGFixedVector2> SGFixedNode2D::get_fixed_scale() {
	return fixed_scale;
}

void SGFixedNode2D::set_fixed_rotation(int64_t p_fixed_rotation) {
	fixed_rotation = p_fixed_rotation;
	updating_transform = true;
	set_rotation(fixed(p_fixed_rotation).to_float());

	SGFixedTransform2DInternal internal_transform = fixed_transform->get_internal();
	internal_transform.set_rotation(fixed(p_fixed_rotation));
	fixed_transform->set_internal(internal_transform);

	updating_transform = false;
	_change_notify("fixed_rotation");
}

int64_t SGFixedNode2D::get_fixed_rotation() const {
	return fixed_rotation;
}

void SGFixedNode2D::set_global_fixed_transform(const Ref<SGFixedTransform2D> &p_global_transform) {
	ERR_FAIL_COND(!p_global_transform.is_valid());
	update_global_fixed_transform_internal(p_global_transform->get_internal());
}

Ref<SGFixedTransform2D> SGFixedNode2D::get_global_fixed_transform() const {
	return Ref<SGFixedTransform2D>(memnew(SGFixedTransform2D(get_global_fixed_transform_internal())));
}

void SGFixedNode2D::set_global_fixed_position(const Ref<SGFixedVector2> &p_fixed_position) {
	ERR_FAIL_COND(!p_fixed_position.is_valid());

	SGFixedTransform2DInternal t = get_global_fixed_transform_internal();
	t.set_origin(p_fixed_position->get_internal());
	update_global_fixed_transform_internal(t);
}

Ref<SGFixedVector2> SGFixedNode2D::get_global_fixed_position() {
	return SGFixedVector2::from_internal(get_global_fixed_transform_internal().get_origin());
}

void SGFixedNode2D::set_global_fixed_rotation(int64_t p_fixed_rotation) {
	SGFixedNode2D *fixed_parent = Object::cast_to<SGFixedNode2D>(get_parent());
	if (fixed_parent) {
		fixed parent_rotation = fixed_parent->get_global_fixed_transform_internal().get_rotation();
		set_fixed_rotation(p_fixed_rotation - parent_rotation.value);
	}
	else {
		set_fixed_rotation(p_fixed_rotation);
	}
}

int64_t SGFixedNode2D::get_global_fixed_rotation() const {
	return get_global_fixed_transform_internal().get_rotation().value;
}

SGFixedNode2D::SGFixedNode2D() {
	fixed_transform = Ref<SGFixedTransform2D>(memnew(SGFixedTransform2D));
	fixed_transform->connect("changed", this, "_fixed_transform_change");

	fixed_position = Ref<SGFixedVector2>(memnew(SGFixedVector2));
	fixed_position->connect("changed", this, "_fixed_position_changed");

	fixed_scale = Ref<SGFixedVector2>(memnew(SGFixedVector2(SGFixedVector2Internal(fixed::ONE, fixed::ONE))));
	fixed_scale->connect("changed", this, "_fixed_scale_changed");

	fixed_rotation = 0;

	updating_transform = false;

	if (Engine::get_singleton()->is_editor_hint()) {
		add_change_receptor(this);
	}
}

SGFixedNode2D::~SGFixedNode2D() {
}