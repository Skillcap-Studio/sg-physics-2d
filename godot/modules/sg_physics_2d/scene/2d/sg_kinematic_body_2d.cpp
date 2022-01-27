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

#include "sg_kinematic_body_2d.h"

#include "../../internal/sg_bodies_2d_internal.h"
#include "../../internal/sg_world_2d_internal.h"

static bool sg_compare_collision_objects(SGCollisionObject2DInternal* p_a, SGCollisionObject2DInternal *p_b) {
	SGCollisionObject2D *a = Object::cast_to<SGCollisionObject2D>((Object *)p_a->get_data());
	SGCollisionObject2D *b = Object::cast_to<SGCollisionObject2D>((Object *)p_b->get_data());
	return b->is_greater_than(a);
}

void SGKinematicBody2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_safe_margin"), &SGKinematicBody2D::get_safe_margin);
	ClassDB::bind_method(D_METHOD("set_safe_margin", "fixed_margin"), &SGKinematicBody2D::set_safe_margin);

	ClassDB::bind_method(D_METHOD("get_floor_normal"), &SGKinematicBody2D::get_floor_normal);
	ClassDB::bind_method(D_METHOD("get_floor_angle", "up_direction"), &SGKinematicBody2D::get_floor_angle, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("is_on_floor"), &SGKinematicBody2D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &SGKinematicBody2D::is_on_ceiling);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &SGKinematicBody2D::is_on_wall);
	ClassDB::bind_method(D_METHOD("get_slide_count"), &SGKinematicBody2D::get_slide_count);

	ClassDB::bind_method(D_METHOD("move_and_collide", "linear_velocity"), &SGKinematicBody2D::_move);
	ClassDB::bind_method(D_METHOD("move_and_slide", "linear_velocity", "up_direction", "unused", "max_slides", "floor_max_angle"), &SGKinematicBody2D::move_and_slide, DEFVAL(Variant()), DEFVAL(false), DEFVAL(4), DEFVAL(51471));
	ClassDB::bind_method(D_METHOD("rotate_and_slide", "rotation", "max_slides"), &SGKinematicBody2D::rotate_and_slide, DEFVAL(4));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "safe_margin"), "set_safe_margin", "get_safe_margin");
}

int SGKinematicBody2D::get_safe_margin() const {
	return safe_margin.value;
}

void SGKinematicBody2D::set_safe_margin(int p_safe_margin) {
	safe_margin = fixed(p_safe_margin);
}

Ref<SGFixedVector2> SGKinematicBody2D::get_floor_normal() const {
	return floor_normal;
}

int SGKinematicBody2D::get_floor_angle(const Ref<SGFixedVector2> &p_up_direction) const {
	SGFixedVector2Internal up_direction(fixed::ZERO, fixed::NEG_ONE);
	if (p_up_direction.is_valid()) {
		up_direction = p_up_direction->get_internal();
	}
	return floor_normal->get_internal().dot(up_direction).acos().value;
}

bool SGKinematicBody2D::is_on_floor() const {
	return on_floor;
}

bool SGKinematicBody2D::is_on_ceiling() const {
	return on_ceiling;
}

bool SGKinematicBody2D::is_on_wall() const {
	return on_wall;
}

int SGKinematicBody2D::get_slide_count() const {
	return colliders.size();
}

bool SGKinematicBody2D::move_and_collide(const SGFixedVector2Internal &p_linear_velocity, SGKinematicBody2D::Collision &p_collision) {
	SGWorld2DInternal *world = SGWorld2DInternal::get_singleton();
	SGWorld2DInternal::BodyOverlapInfo overlap_info;

	// First, get our body unstuck, if it's stuck.
	bool stuck = world->get_best_overlapping_body(internal, safe_margin, &overlap_info, &sg_compare_collision_objects);
	if (stuck) {
		for (int i = 0; i < 4; i++) {
			SGFixedTransform2DInternal t = internal->get_transform();
			t.set_origin(t.get_origin() + overlap_info.separation);
			internal->set_transform(t);

			stuck = world->get_best_overlapping_body(internal, safe_margin, &overlap_info, &sg_compare_collision_objects);
			if (!stuck) {
				break;
			}
		}
	}
	if (stuck) {
		set_global_fixed_position_internal(internal->get_transform().get_origin());

		// We can't really continue. Bail with some sort of reasonable values.
		p_collision.collider = Object::cast_to<SGCollisionObject2D>((Object *)overlap_info.collider->get_data());
		p_collision.normal = SGFixedVector2Internal::ZERO;
		p_collision.remainder = p_linear_velocity;
		return true;
	}

	// Move the body the full amount.
	SGFixedTransform2DInternal original_transform = internal->get_transform();
	SGFixedTransform2DInternal test_transform = original_transform;
	SGFixedVector2Internal destination = original_transform.get_origin() + p_linear_velocity;
	test_transform.set_origin(destination);
	internal->set_transform(test_transform);

	// Check if we're colliding. If not, sync from physics engine and bail.
	if (!world->get_best_overlapping_body(internal, fixed::ZERO, &overlap_info, &sg_compare_collision_objects)) {
		set_global_fixed_position_internal(test_transform.get_origin());
		return false;
	}
	
	// Use binary search to find the point at which we collide, and the point just before that.
	fixed low = fixed::ZERO;
	fixed hi = fixed::ONE;
	for (int i = 0; i < 8; i++) {
		fixed cur = (low + hi) * fixed::HALF;
		SGFixedVector2Internal test_position = original_transform.get_origin() + (p_linear_velocity * cur);
		test_transform.set_origin(test_position);
		internal->set_transform(test_transform);
		if (world->get_best_overlapping_body(internal, fixed::ZERO, &overlap_info, &sg_compare_collision_objects)) {
			hi = cur;
		}
		else {
			low = cur;
			set_global_fixed_position_internal(test_position);
		}
	}

	// Whatever was last set to our fixed position will be a safe position, so
	// let's make sure that's what ends up in the physics engine (in case the
	// last test_transform was a hi position).
	sync_to_physics_engine();

	// At this point, the overlap_info will contain info about the collision at 'hi'
	// which is what we want to store in p_collision.
	p_collision.collider = Object::cast_to<SGCollisionObject2D>((Object *)overlap_info.collider->get_data());
	p_collision.normal = overlap_info.collision_normal;
	p_collision.remainder = p_linear_velocity - (p_linear_velocity * low);

	return true;
}

Ref<SGFixedVector2> SGKinematicBody2D::move_and_slide(const Ref<SGFixedVector2> &p_linear_velocity, const Ref<SGFixedVector2> &p_up_direction, bool p_unused, int p_max_slides, int p_floor_max_angle) {
	ERR_FAIL_COND_V(!p_linear_velocity.is_valid(), Ref<SGFixedVector2>());
	
	SGFixedVector2Internal motion = p_linear_velocity->get_internal();
	SGFixedVector2Internal up_direction;
	fixed floor_max_angle = fixed(p_floor_max_angle);

	if (p_up_direction.is_valid()) {
		up_direction = p_up_direction->get_internal();
	}

	colliders.clear();
	floor_normal->clear();
	on_floor = false;
	on_ceiling = false;
	on_wall = false;

	while (p_max_slides) {
		Collision collision;

		if (!move_and_collide(motion, collision)) {
			// No collision, so we're good - bail!
			break;
		}

		if (collision.normal == SGFixedVector2Internal::ZERO) {
			// This means we couldn't unstuck the body. Clear out the motion
			// vector and bail.
			motion = SGFixedVector2Internal::ZERO;
			break;
		}

		colliders.push_back(collision.collider);

		if (up_direction == SGFixedVector2Internal::ZERO) {
			// All is wall!
			on_wall = true;
		}
		else {
			if (collision.normal.dot(up_direction).acos() <= floor_max_angle) {
				on_floor = true;
				floor_normal->set_internal(collision.normal);
			}
			else if (collision.normal.dot(-up_direction).acos() <= floor_max_angle) {
				on_ceiling = true;
			}
			else {
				on_wall = true;
			}
		}

		motion = collision.remainder.slide(collision.normal);

		if (motion == SGFixedVector2Internal::ZERO) {
			// No remaining motion, so we're good - bail!
			break;
		}

		p_max_slides--;
	}

	return Ref<SGFixedVector2>(memnew(SGFixedVector2(motion)));
}

bool SGKinematicBody2D::rotate_and_slide(int64_t p_rotation, int p_max_slides) {
	SGWorld2DInternal *world = SGWorld2DInternal::get_singleton();
	SGWorld2DInternal::BodyOverlapInfo overlap_info;

	set_fixed_rotation(get_fixed_rotation() + p_rotation);
	sync_to_physics_engine();

	bool stuck = world->get_best_overlapping_body(internal, fixed::ZERO, &overlap_info, &sg_compare_collision_objects);
	if (stuck) {
		for (int i = 0; i < p_max_slides; i++) {
			SGFixedTransform2DInternal t = internal->get_transform();
			t.set_origin(t.get_origin() + overlap_info.separation);
			internal->set_transform(t);

			stuck = world->get_best_overlapping_body(internal, fixed::ZERO, &overlap_info, &sg_compare_collision_objects);
			if (!stuck) {
				break;
			}
		}
	}

	set_global_fixed_position_internal(internal->get_transform().get_origin());

	return stuck;
}

Ref<SGKinematicCollision2D> SGKinematicBody2D::_move(const Ref<SGFixedVector2> &p_linear_velocity) {
	SGKinematicBody2D::Collision collision;
	if (move_and_collide(p_linear_velocity->get_internal(), collision)) {
		Ref<SGKinematicCollision2D> result = Ref<SGKinematicCollision2D>(memnew(SGKinematicCollision2D));
		result->set_collision(collision);
		return result;
	}
	return Ref<SGKinematicCollision2D>();
}

SGKinematicBody2D::SGKinematicBody2D()
	: SGCollisionObject2D(memnew(SGBody2DInternal(SGBody2DInternal::BodyType::BODY_KINEMATIC)))
{
	safe_margin = fixed(64);
	floor_normal.instance();
	on_floor = false;
	on_ceiling = false;
	on_wall = false;
}

SGKinematicBody2D::~SGKinematicBody2D() {
}

void SGKinematicCollision2D::set_collision(const SGKinematicBody2D::Collision &p_collision) {
	collision = p_collision;
	normal->set_internal(collision.normal);
	remainder->set_internal(collision.remainder);
}

void SGKinematicCollision2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_collider"), &SGKinematicCollision2D::get_collider);
	ClassDB::bind_method(D_METHOD("get_normal"), &SGKinematicCollision2D::get_normal);
	ClassDB::bind_method(D_METHOD("get_remainder"), &SGKinematicCollision2D::get_remainder);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider"), "", "get_collider");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal", PROPERTY_HINT_TYPE_STRING, "SGFixedVector2"), "", "get_normal");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "remainder", PROPERTY_HINT_TYPE_STRING, "SGFixedVector2"), "", "get_remainder");
}

Object *SGKinematicCollision2D::get_collider() const {
	return collision.collider;
}

Ref<SGFixedVector2> SGKinematicCollision2D::get_normal() const {
	return normal;
}

Ref<SGFixedVector2> SGKinematicCollision2D::get_remainder() const {
	return remainder;
}

SGKinematicCollision2D::SGKinematicCollision2D() {
	normal = Ref<SGFixedVector2>(memnew(SGFixedVector2));
	remainder = Ref<SGFixedVector2>(memnew(SGFixedVector2));
}
