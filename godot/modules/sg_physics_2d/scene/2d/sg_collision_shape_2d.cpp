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

#include "sg_collision_shape_2d.h"

#include <core/engine.h>

#include "../../internal/sg_shapes_2d_internal.h"

void SGCollisionShape2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &SGCollisionShape2D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &SGCollisionShape2D::get_shape);

	ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &SGCollisionShape2D::set_disabled);
	ClassDB::bind_method(D_METHOD("get_disabled"), &SGCollisionShape2D::get_disabled);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "SGShape2D"), "set_shape", "get_shape");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "get_disabled");

	ClassDB::bind_method(D_METHOD("_shape_changed"), &SGCollisionShape2D::_shape_changed);
}

void SGCollisionShape2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
				break;
			}

			if (!shape.is_valid()) {
				break;
			}

			Color draw_col = get_tree()->get_debug_collisions_color();
			/*
			if (disabled) {
				float g = draw_col.get_v();
				draw_col.r = g;
				draw_col.g = g;
				draw_col.b = g;
				draw_col.a *= 0.5;
			}
			*/

			shape->draw(get_canvas_item(), draw_col);
		} break;
		
		case NOTIFICATION_PARENTED:
			collision_object = Object::cast_to<SGCollisionObject2D>(get_parent());
			if (collision_object && internal_shape && !disabled) {
				collision_object->add_shape(internal_shape);
			}
			break;
		
		case NOTIFICATION_UNPARENTED:
			if (collision_object && internal_shape && !disabled) {
				collision_object->remove_shape(internal_shape);
			}
			collision_object = nullptr;
			break;

	}
}

void SGCollisionShape2D::set_disabled(bool p_disabled) {
	if (disabled != p_disabled) {
		disabled = p_disabled;
		if (collision_object && internal_shape) {
			if (disabled) {
				collision_object->remove_shape(internal_shape);
			}
			else {
				collision_object->add_shape(internal_shape);
			}
		}
	}
}

bool SGCollisionShape2D::get_disabled() const {
	return disabled;
}

void SGCollisionShape2D::set_shape(const Ref<SGShape2D> &p_shape) {
	if (shape.is_valid()) {
		shape->disconnect("changed", this, "_shape_changed");
		if (collision_object && internal_shape && !disabled) {
			collision_object->remove_shape(internal_shape);
			memdelete(internal_shape);
			internal_shape = nullptr;
		}
	}

	shape = p_shape;

	if (shape.is_valid()) {
		shape->connect("changed", this, "_shape_changed");
		internal_shape = shape->create_internal_shape();
		if (collision_object && !disabled) {
			collision_object->add_shape(internal_shape);
		}
	}

	update();
}

Ref<SGShape2D> SGCollisionShape2D::get_shape() {
	return shape;
}

void SGCollisionShape2D::_shape_changed() {
	update();
}

void SGCollisionShape2D::sync_to_physics_engine() const {
	if (shape.is_valid() && internal_shape && !disabled) {
		internal_shape->set_transform(get_fixed_transform_internal());
		shape->sync_to_physics_engine(internal_shape);
	}
}

SGCollisionShape2D::SGCollisionShape2D() {
	disabled = false;
	collision_object = nullptr;
	internal_shape = nullptr;
}

SGCollisionShape2D::~SGCollisionShape2D() {
	if (collision_object && internal_shape && !disabled) {
		collision_object->remove_shape(internal_shape);
	}
	if (internal_shape) {
		memdelete(internal_shape);
	}
}
