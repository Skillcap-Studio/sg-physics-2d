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

#ifndef SG_KINEMATIC_BODY_2D_H
#define SG_KINEMATIC_BODY_2D_H

#include "sg_collision_object_2d.h"

class SGKinematicCollision2D;

class SGKinematicBody2D : public SGCollisionObject2D {
	GDCLASS(SGKinematicBody2D, SGCollisionObject2D);

protected:
	Vector<Ref<SGKinematicCollision2D>> slide_colliders;
	Ref<SGFixedVector2> floor_normal;
	bool on_floor;
	bool on_ceiling;
	bool on_wall;

	static void _bind_methods();

public:
	int get_safe_margin() const;
	void set_safe_margin(int p_safe_margin);

	Ref<SGFixedVector2> get_floor_normal() const;
	int get_floor_angle(const Ref<SGFixedVector2> &p_up_direction) const;
	bool is_on_floor() const;
	bool is_on_ceiling() const;
	bool is_on_wall() const;

	int get_slide_count() const;
	Ref<SGKinematicCollision2D> get_slide_collision(int p_slide_idx);
	Ref<SGKinematicCollision2D> get_last_slide_collision();

	Ref<SGKinematicCollision2D> move_and_collide(const Ref<SGFixedVector2> &p_linear_velocity);
	Ref<SGFixedVector2> move_and_slide(const Ref<SGFixedVector2> &p_linear_velocity, const Ref<SGFixedVector2> &p_up_direction, bool p_unused, int p_max_slides, int p_floor_max_angle);
	bool rotate_and_slide(int64_t p_rotation, int p_max_slides);

	SGKinematicBody2D();
	~SGKinematicBody2D();
};

#endif
