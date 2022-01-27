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

#include "sg_world_2d_internal.h"

#include <core/project_settings.h>

#include "sg_bodies_2d_internal.h"
#include "sg_shapes_2d_internal.h"
#include "sg_broadphase_2d_internal.h"
#include "sg_collision_detector_2d_internal.h"

SGWorld2DInternal *SGWorld2DInternal::singleton = NULL;

SGWorld2DInternal *SGWorld2DInternal::get_singleton() {
	return singleton;
}

void SGWorld2DInternal::add_area(SGArea2DInternal *p_area) {
	areas.push_back(p_area);
	p_area->add_to_broadphase(broadphase);
}

void SGWorld2DInternal::remove_area(SGArea2DInternal *p_area) {
	areas.erase(p_area);
	p_area->remove_from_broadphase();
}

void SGWorld2DInternal::add_body(SGBody2DInternal *p_body) {
	bodies.push_back(p_body);
	p_body->add_to_broadphase(broadphase);
}

void SGWorld2DInternal::remove_body(SGBody2DInternal *p_body) {
	bodies.erase(p_body);
	p_body->remove_from_broadphase();
}

bool SGWorld2DInternal::overlaps(SGCollisionObject2DInternal *p_object1, SGCollisionObject2DInternal *p_object2, fixed p_margin, SGWorld2DInternal::BodyOverlapInfo *p_info) const {
	bool overlapping = false;

	SGWorld2DInternal::ShapeOverlapInfo shape_overlap_info;
	fixed longest_separation_squared;

	for (const List<SGShape2DInternal *>::Element *S1 = p_object1->get_shapes().front(); S1; S1 = S1->next()) {
		for (const List<SGShape2DInternal *>::Element *S2 = p_object2->get_shapes().front(); S2; S2 = S2->next()) {
			if (overlaps(S1->get(), S2->get(), p_margin, &shape_overlap_info)) {
				overlapping = true;
				if (!p_info) {
					return overlapping;
				}

				fixed separation_length_squared = shape_overlap_info.separation.length_squared();
				if (separation_length_squared > longest_separation_squared) {
					longest_separation_squared = separation_length_squared;
					p_info->collider = p_object2;
					p_info->collider_shape = S2->get();
					p_info->local_shape = S1->get();
					p_info->separation = shape_overlap_info.separation;
					p_info->collision_normal = shape_overlap_info.collision_normal;
				}
			}
		}
	}

	return overlapping;
}

bool SGWorld2DInternal::overlaps(SGShape2DInternal *p_shape1, SGShape2DInternal *p_shape2, fixed p_margin, SGWorld2DInternal::ShapeOverlapInfo *p_info) const {
	using ShapeType = SGShape2DInternal::ShapeType;

	ShapeType shape1_type = p_shape1->get_shape_type();
	ShapeType shape2_type = p_shape2->get_shape_type();

	SGCollisionDetector2DInternal::OverlapInfo overlap_info;
	SGCollisionDetector2DInternal::OverlapInfo *overlap_info_ptr = p_info ? &overlap_info : nullptr;

	bool overlapping = false;
	bool swap = false;

	if (shape1_type == ShapeType::SHAPE_RECTANGLE && shape2_type == ShapeType::SHAPE_RECTANGLE) {
		overlapping = SGCollisionDetector2DInternal::Rectangle_overlaps_Rectangle(*((SGRectangle2DInternal *)p_shape1), *((SGRectangle2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_CIRCLE && shape2_type == ShapeType::SHAPE_CIRCLE) {
		overlapping = SGCollisionDetector2DInternal::Circle_overlaps_Circle(*((SGCircle2DInternal *)p_shape1), *((SGCircle2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_CIRCLE && shape2_type == ShapeType::SHAPE_RECTANGLE) {
		overlapping = SGCollisionDetector2DInternal::Circle_overlaps_Rectangle(*((SGCircle2DInternal *)p_shape1), *((SGRectangle2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_RECTANGLE && shape2_type == ShapeType::SHAPE_CIRCLE) {
		overlapping = SGCollisionDetector2DInternal::Circle_overlaps_Rectangle(*((SGCircle2DInternal *)p_shape2), *((SGRectangle2DInternal *)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_POLYGON && shape2_type == ShapeType::SHAPE_POLYGON) {
		overlapping = SGCollisionDetector2DInternal::Polygon_overlaps_Polygon(*((SGPolygon2DInternal *)p_shape1), *((SGPolygon2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_POLYGON && shape2_type == ShapeType::SHAPE_CIRCLE) {
		overlapping = SGCollisionDetector2DInternal::Polygon_overlaps_Circle(*((SGPolygon2DInternal *)p_shape1), *((SGCircle2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_CIRCLE && shape2_type == ShapeType::SHAPE_POLYGON) {
		overlapping = SGCollisionDetector2DInternal::Polygon_overlaps_Circle(*((SGPolygon2DInternal *)p_shape2), *((SGCircle2DInternal *)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_POLYGON && shape2_type == ShapeType::SHAPE_RECTANGLE) {
		overlapping = SGCollisionDetector2DInternal::Polygon_overlaps_Rectangle(*((SGPolygon2DInternal *)p_shape1), *((SGRectangle2DInternal *)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_RECTANGLE && shape2_type == ShapeType::SHAPE_POLYGON) {
		overlapping = SGCollisionDetector2DInternal::Polygon_overlaps_Rectangle(*((SGPolygon2DInternal *)p_shape2), *((SGRectangle2DInternal *)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_CAPSULE && shape2_type == ShapeType::SHAPE_CIRCLE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Circle(*((SGCapsule2DInternal*)p_shape1), *((SGCircle2DInternal*)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_CIRCLE && shape2_type == ShapeType::SHAPE_CAPSULE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Circle(*((SGCapsule2DInternal*)p_shape2), *((SGCircle2DInternal*)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_CAPSULE && shape2_type == ShapeType::SHAPE_RECTANGLE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Rectangle(*((SGCapsule2DInternal*)p_shape1), *((SGRectangle2DInternal*)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_RECTANGLE && shape2_type == ShapeType::SHAPE_CAPSULE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Rectangle(*((SGCapsule2DInternal*)p_shape2), *((SGRectangle2DInternal*)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_CAPSULE && shape2_type == ShapeType::SHAPE_POLYGON) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Polygon(*((SGCapsule2DInternal*)p_shape1), *((SGPolygon2DInternal*)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_POLYGON && shape2_type == ShapeType::SHAPE_CAPSULE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Polygon(*((SGCapsule2DInternal*)p_shape2), *((SGPolygon2DInternal*)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}
	else if (shape1_type == ShapeType::SHAPE_CAPSULE && shape2_type == ShapeType::SHAPE_CAPSULE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Capsule(*((SGCapsule2DInternal*)p_shape1), *((SGCapsule2DInternal*)p_shape2), p_margin, overlap_info_ptr);
	}
	else if (shape1_type == ShapeType::SHAPE_CAPSULE && shape2_type == ShapeType::SHAPE_CAPSULE) {
		overlapping = SGCollisionDetector2DInternal::Capsule_overlaps_Capsule(*((SGCapsule2DInternal*)p_shape2), *((SGCapsule2DInternal*)p_shape1), p_margin, overlap_info_ptr);
		swap = true;
	}

	if (overlapping && p_info) {
		// Make sure the info is from the perspective of the first shape.
		p_info->shape = p_shape2;
		p_info->collision_normal = swap ? -overlap_info.collision_normal : overlap_info.collision_normal;
		p_info->separation = swap ? -overlap_info.separation : overlap_info.separation;
	}

	return overlapping;
}

class SGBestOverlappingResultHandler : public SGResultHandlerInternal {
private:

	const SGWorld2DInternal *world;
	SGCollisionObject2DInternal *object;
	fixed margin;
	SGWorld2DInternal::BodyOverlapInfo *overlap_info;
	SGWorld2DInternal::CompareCallback compare;

	bool overlapping;
	SGWorld2DInternal::BodyOverlapInfo test_overlap_info;
	fixed longest_separation_squared;

public:

	void handle_result(SGCollisionObject2DInternal *p_object) {
		if (object == p_object) {
			return;
		}

		SGBody2DInternal *other = (SGBody2DInternal *)p_object;
		if (!object->test_collision_layers(other)) {
			return;
		}

		if (world->overlaps(object, other, margin, &test_overlap_info)) {
			overlapping = true;

			fixed separation_length_squared = test_overlap_info.separation.length_squared();
			if (separation_length_squared > longest_separation_squared) {
				longest_separation_squared = separation_length_squared;
				*overlap_info = test_overlap_info;
			}
			// If we find another with the same separation, use the compare
			// callback to decide which is first.
			else if (separation_length_squared == longest_separation_squared && compare != nullptr && overlap_info->collider != nullptr) {
				if (compare(other, overlap_info->collider)) {
					*overlap_info = test_overlap_info;
				}
			}
		}
	}

	_FORCE_INLINE_ bool is_overlapping() const { return overlapping; }

	_FORCE_INLINE_ SGBestOverlappingResultHandler(const SGWorld2DInternal *p_world, SGCollisionObject2DInternal *p_object, fixed p_margin, SGWorld2DInternal::BodyOverlapInfo *p_overlap_info, SGWorld2DInternal::CompareCallback p_compare)
		: world(p_world), object(p_object), margin(p_margin), overlap_info(p_overlap_info), compare(p_compare), overlapping(false) { }

};

bool SGWorld2DInternal::get_best_overlapping_body(SGCollisionObject2DInternal *p_object, fixed p_margin, SGWorld2DInternal::BodyOverlapInfo *p_info, SGWorld2DInternal::CompareCallback p_compare) const {
	SGFixedRect2Internal bounds = p_object->get_bounds();
	bounds.grow_by(p_margin);

	SGBestOverlappingResultHandler result_handler(this, p_object, p_margin, p_info, p_compare);
	broadphase->find_nearby(bounds, &result_handler, SGCollisionObject2DInternal::OBJECT_BODY);
	return result_handler.is_overlapping();
}

class SGOverlappingResultHandler : public SGResultHandlerInternal {
private:

	const SGWorld2DInternal *world;
	SGCollisionObject2DInternal *object;
	SGResultHandlerInternal *result_handler;

public:

	void handle_result(SGCollisionObject2DInternal *p_object) {
		if (object == p_object) {
			return;
		}

		if (!object->test_collision_layers(p_object)) {
			return;
		}

		if (world->overlaps(object, p_object, fixed::ZERO)) {
			result_handler->handle_result(p_object);
		}
	}

	_FORCE_INLINE_ SGOverlappingResultHandler(const SGWorld2DInternal *p_world, SGCollisionObject2DInternal *p_object, SGResultHandlerInternal *p_result_handler)
		: world(p_world), object(p_object), result_handler(p_result_handler) { }

};

void SGWorld2DInternal::get_overlapping_areas(SGCollisionObject2DInternal *p_object, SGResultHandlerInternal *p_result_handler) const {
	SGOverlappingResultHandler overlapping_handler(this, p_object, p_result_handler);
	broadphase->find_nearby(p_object->get_bounds(), &overlapping_handler, SGCollisionObject2DInternal::OBJECT_AREA);
}

void SGWorld2DInternal::get_overlapping_bodies(SGCollisionObject2DInternal *p_object, SGResultHandlerInternal *p_result_handler) const {
	SGOverlappingResultHandler overlapping_handler(this, p_object, p_result_handler);
	broadphase->find_nearby(p_object->get_bounds(), &overlapping_handler, SGCollisionObject2DInternal::OBJECT_BODY);
}

bool SGWorld2DInternal::segment_intersects_shape(const SGFixedVector2Internal &p_start, const SGFixedVector2Internal &p_cast_to, SGShape2DInternal *p_shape, SGFixedVector2Internal &p_intersection_point, SGFixedVector2Internal &p_collision_normal) const {
	using ShapeType = SGShape2DInternal::ShapeType;

	ShapeType shape_type = p_shape->get_shape_type();

	switch (shape_type) {
		case ShapeType::SHAPE_RECTANGLE:
		case ShapeType::SHAPE_POLYGON:
			return SGCollisionDetector2DInternal::segment_intersects_Polygon(p_start, p_cast_to, *(SGShape2DInternal *)p_shape, p_intersection_point, p_collision_normal);
		
		case ShapeType::SHAPE_CIRCLE:
			return SGCollisionDetector2DInternal::segment_intersects_Circle(p_start, p_cast_to, *(SGCircle2DInternal *)p_shape, p_intersection_point, p_collision_normal);

		case ShapeType::SHAPE_CAPSULE:
			return SGCollisionDetector2DInternal::segment_intersects_Capsule(p_start, p_cast_to, *(SGCapsule2DInternal*)p_shape, p_intersection_point, p_collision_normal);
	}

	return false;
}

class SGRayCastResultHandler : public SGResultHandlerInternal {
private:

	const SGWorld2DInternal *world;
	const SGFixedVector2Internal &start;
	const SGFixedVector2Internal &cast_to;
	uint32_t collision_mask;
	Set<SGCollisionObject2DInternal *> *exceptions;

	bool intersects;
	SGCollisionObject2DInternal *collider;
	fixed shortest_distance_squared;
	SGFixedVector2Internal closest_intersection_point;
	SGFixedVector2Internal closest_collision_normal;
	SGFixedVector2Internal intersection_point;
	SGFixedVector2Internal collision_normal;

public:

	void handle_result(SGCollisionObject2DInternal *p_object) {
		if (exceptions && exceptions->has(p_object)) {
			return;
		}

		if (!(p_object->get_collision_layer() & collision_mask)) {
			return;
		}

		for (const List<SGShape2DInternal *>::Element *S = p_object->get_shapes().front(); S; S = S->next()) {
			SGShape2DInternal *shape = S->get();
			if (world->segment_intersects_shape(start, cast_to, shape, intersection_point, collision_normal)) {
				intersects = true;

				fixed distance_squared = (intersection_point - start).length_squared();
				if (collider == nullptr || distance_squared < shortest_distance_squared) {
					shortest_distance_squared = distance_squared;
					collider = p_object;
					closest_intersection_point = intersection_point;
					closest_collision_normal = collision_normal;
				}
			}
		}
	}

	_FORCE_INLINE_ void populate_info(SGWorld2DInternal::RayCastInfo *p_info) {
		if (collider) {
			p_info->body = (SGBody2DInternal *)collider;
			p_info->collision_point = closest_intersection_point;
			p_info->collision_normal = closest_collision_normal;
		}
	}

	_FORCE_INLINE_ bool is_intersecting() const {
		return intersects;
	}

	_FORCE_INLINE_ SGRayCastResultHandler(const SGWorld2DInternal *p_world, const SGFixedVector2Internal &p_start, const SGFixedVector2Internal &p_cast_to, uint32_t p_collision_mask, Set<SGCollisionObject2DInternal *> *p_exceptions)
		: world(p_world), start(p_start), cast_to(p_cast_to), collision_mask(p_collision_mask), exceptions(p_exceptions), intersects(false), collider(nullptr) {
		SGFixedRect2Internal bounds(p_start, SGFixedVector2Internal());
		bounds.expand_to(p_start + p_cast_to);
	}

};

bool SGWorld2DInternal::cast_ray(const SGFixedVector2Internal &p_start, const SGFixedVector2Internal &p_cast_to, uint32_t p_collision_mask, Set<SGCollisionObject2DInternal *> *p_exceptions, SGWorld2DInternal::RayCastInfo *p_info) const {
	SGRayCastResultHandler result_handler(this, p_start, p_cast_to, p_collision_mask, p_exceptions);

	SGFixedRect2Internal bounds(p_start, SGFixedVector2Internal());
	bounds.expand_to(p_start + p_cast_to);

	broadphase->find_nearby(bounds, &result_handler, SGCollisionObject2DInternal::OBJECT_BODY);
	if (p_info) {
		result_handler.populate_info(p_info);
	}
	return result_handler.is_intersecting();
}

SGWorld2DInternal::SGWorld2DInternal()
{
	int cell_size = ProjectSettings::get_singleton()->get_setting("physics/2d/cell_size");
	if (cell_size == 0) {
		cell_size = 128;
	}

	broadphase = memnew(SGBroadphase2DInternal(cell_size));
	singleton = this;
}

SGWorld2DInternal::~SGWorld2DInternal() {
	memdelete(broadphase);
	singleton = nullptr;
}
