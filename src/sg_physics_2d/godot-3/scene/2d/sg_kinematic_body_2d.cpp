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

#include "../../servers/sg_physics_2d_server.h"
#include "../../../internal/sg_bodies_2d_internal.h"
#include "../../../internal/sg_world_2d_internal.h"

void SGKinematicBody2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_safe_margin"), &SGKinematicBody2D::get_safe_margin);
	ClassDB::bind_method(D_METHOD("set_safe_margin", "fixed_margin"), &SGKinematicBody2D::set_safe_margin);

	ClassDB::bind_method(D_METHOD("get_floor_normal"), &SGKinematicBody2D::get_floor_normal);
	ClassDB::bind_method(D_METHOD("get_floor_angle", "up_direction"), &SGKinematicBody2D::get_floor_angle, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("is_on_floor"), &SGKinematicBody2D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &SGKinematicBody2D::is_on_ceiling);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &SGKinematicBody2D::is_on_wall);

	ClassDB::bind_method(D_METHOD("get_slide_count"), &SGKinematicBody2D::get_slide_count);
	ClassDB::bind_method(D_METHOD("get_slide_collision", "slide_idx"), &SGKinematicBody2D::get_slide_collision);
	ClassDB::bind_method(D_METHOD("get_last_slide_collision"), &SGKinematicBody2D::get_last_slide_collision);

	ClassDB::bind_method(D_METHOD("move_and_collide", "linear_velocity"), &SGKinematicBody2D::move_and_collide);
	ClassDB::bind_method(D_METHOD("move_and_slide", "linear_velocity", "up_direction", "unused", "max_slides", "floor_max_angle"), &SGKinematicBody2D::move_and_slide, DEFVAL(Variant()), DEFVAL(false), DEFVAL(4), DEFVAL(51471));
	ClassDB::bind_method(D_METHOD("rotate_and_slide", "rotation", "max_slides"), &SGKinematicBody2D::rotate_and_slide, DEFVAL(4));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "safe_margin"), "set_safe_margin", "get_safe_margin");
}

int SGKinematicBody2D::get_safe_margin() const {
	return SGPhysics2DServer::get_singleton()->body_get_safe_margin(rid);
}

void SGKinematicBody2D::set_safe_margin(int p_safe_margin) {
	return SGPhysics2DServer::get_singleton()->body_set_safe_margin(rid, p_safe_margin);
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
	return slide_colliders.size();
}

Ref<SGKinematicCollision2D> SGKinematicBody2D::get_slide_collision(int p_bounce) {
	ERR_FAIL_INDEX_V(p_bounce, slide_colliders.size(), Ref<SGKinematicCollision2D>());
	return slide_colliders[p_bounce];
}

Ref<SGKinematicCollision2D> SGKinematicBody2D::get_last_slide_collision() {
	if (slide_colliders.size() == 0) {
		return Ref<SGKinematicCollision2D>();
	}
	return get_slide_collision(slide_colliders.size() - 1);
}

Ref<SGKinematicCollision2D> SGKinematicBody2D::move_and_collide(const Ref<SGFixedVector2> &p_linear_velocity) {
	ERR_FAIL_COND_V(!p_linear_velocity.is_valid(), Ref<SGKinematicCollision2D>());
	SGPhysics2DServer *physics_server = SGPhysics2DServer::get_singleton();
	Ref<SGKinematicCollision2D> ret = physics_server->body_move_and_collide(rid, p_linear_velocity);
	// Sync only position from physics server to prevent precision loss.
	set_global_fixed_position_internal(physics_server->collision_object_get_internal(rid)->get_transform().get_origin());
	return ret;
}

Ref<SGFixedVector2> SGKinematicBody2D::move_and_slide(const Ref<SGFixedVector2> &p_linear_velocity, const Ref<SGFixedVector2> &p_up_direction, bool p_unused, int p_max_slides, int p_floor_max_angle) {
	ERR_FAIL_COND_V(!p_linear_velocity.is_valid(), Ref<SGFixedVector2>());

	SGPhysics2DServer *physics_server = SGPhysics2DServer::get_singleton();

	SGFixedVector2Internal motion_internal = p_linear_velocity->get_internal();
	SGFixedVector2Internal body_velocity = motion_internal;
	SGFixedVector2Internal up_direction;
	fixed floor_max_angle = fixed(p_floor_max_angle);

	if (p_up_direction.is_valid()) {
		up_direction = p_up_direction->get_internal();
	}

	slide_colliders.clear();
	floor_normal->clear();
	on_floor = false;
	on_ceiling = false;
	on_wall = false;

	Ref<SGFixedVector2> motion;
	motion.instance();

	while (p_max_slides) {
		motion->set_internal(motion_internal);
		Ref<SGKinematicCollision2D> collision = physics_server->body_move_and_collide(rid, motion);
		if (collision.is_null()) {
			// No collision, so we're good - bail!
			break;
		}

		SGFixedVector2Internal collision_normal = collision->get_normal()->get_internal();
		SGFixedVector2Internal collision_remainder = collision->get_remainder()->get_internal();

		if (collision_normal == SGFixedVector2Internal::ZERO) {
			// This means we couldn't unstuck the body. Clear out the motion
			// vector and bail.
			body_velocity = SGFixedVector2Internal::ZERO;
			break;
		}

		slide_colliders.push_back(collision);

		if (up_direction == SGFixedVector2Internal::ZERO) {
			// All is wall!
			on_wall = true;
		}
		else {
			if (collision_normal.dot(up_direction).acos() <= floor_max_angle) {
				on_floor = true;
				floor_normal->set_internal(collision_normal);
			} else if (collision_normal.dot(-up_direction).acos() <= floor_max_angle) {
				on_ceiling = true;
			} else {
				on_wall = true;
			}
		}

		motion_internal = collision_remainder.slide(collision_normal);
		body_velocity = body_velocity.slide(collision_normal);

		if (motion_internal == SGFixedVector2Internal::ZERO) {
			// No remaining motion, so we're good - bail!
			break;
		}

		p_max_slides--;
	}

	// Sync only position from physics server to prevent precision loss.
	set_global_fixed_position_internal(physics_server->collision_object_get_internal(rid)->get_transform().get_origin());

	return Ref<SGFixedVector2>(memnew(SGFixedVector2(body_velocity)));
}

bool SGKinematicBody2D::rotate_and_slide(int64_t p_rotation, int p_max_slides) {
	set_fixed_rotation(get_fixed_rotation() + p_rotation);

	sync_to_physics_engine();
	bool stuck = SGPhysics2DServer::get_singleton()->body_unstuck(rid, p_max_slides);

	// Sync only position from physics server to prevent precision loss.
	set_global_fixed_position_internal(SGPhysics2DServer::get_singleton()->collision_object_get_internal(rid)->get_transform().get_origin());

	return stuck;
}

SGKinematicBody2D::SGKinematicBody2D()
	: SGCollisionObject2D(SGPhysics2DServer::get_singleton()->collision_object_create(SGPhysics2DServer::OBJECT_BODY, SGPhysics2DServer::BODY_KINEMATIC))
{
	floor_normal.instance();
	on_floor = false;
	on_ceiling = false;
	on_wall = false;
}

SGKinematicBody2D::~SGKinematicBody2D() {
}

