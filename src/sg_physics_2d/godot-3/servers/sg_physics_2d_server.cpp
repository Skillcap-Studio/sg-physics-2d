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

#include "sg_physics_2d_server.h"

#include "core/method_bind_ext.gen.inc"
#include "scene/main/node.h"

#include "../scene/2d/sg_collision_object_2d.h"
#include "../../internal/sg_shapes_2d_internal.h"
#include "../../internal/sg_bodies_2d_internal.h"
#include "../../internal/sg_world_2d_internal.h"

struct SGInternalData {
	RID rid;
	Variant data;

	SGInternalData(RID p_rid) : rid(p_rid) {}

	bool is_greater_than(const SGInternalData &p_other) {
		Variant::Type a_t = data.get_type();
		Variant::Type b_t = p_other.data.get_type();
		if (a_t == Variant::NIL || b_t == Variant::NIL) {
			WARN_PRINT("Can't sort physics objects deterministically if data is null");
		}
		if (a_t == b_t) {
			if (a_t == Variant::OBJECT) {
				// These have to be Node's because we check when setting the data.
				Node *a = Object::cast_to<Node>(data);
				Node *b = Object::cast_to<Node>(p_other.data);
				return a->is_greater_than(b);
			}
			if (a_t == Variant::STRING) {
				String a = data;
				String b = p_other.data;
				return a.casecmp_to(b) == 1;
			}
			return true;
		}

		if (a_t == Variant::OBJECT) {
			// Push objects to the start of the list.
			return false;
		}
		if (a_t == Variant::STRING) {
			// Put strings after objects.
			return b_t == Variant::OBJECT ? true : false;
		}
		if (a_t == Variant::NIL) {
			// Put null after strings.
			return true;
		}

		// This should never be reached, but if anything else gets in there,
		// then push it to the end of the list.
		return true;
	}

	Object *get_object() {
		if (data.get_type() == Variant::OBJECT) {
			return (Object *)data;
		}
		return nullptr;
	}
};

SGPhysics2DServer *SGPhysics2DServer::singleton = nullptr;

static bool sg_compare_collision_objects(const SGCollisionObject2DInternal* p_a, const SGCollisionObject2DInternal *p_b) {
	SGInternalData *a = (SGInternalData *)p_a->get_data();
	SGInternalData *b = (SGInternalData *)p_b->get_data();
	return b->is_greater_than(*a);
}

struct SGCollisionObjectComparator {
	bool operator()(const SGCollisionObject2DInternal *p_a, const SGCollisionObject2DInternal *p_b) const {
		return sg_compare_collision_objects(p_a, p_b);
	}
};

struct SGAreaCollision2DComparator {
	bool operator()(const SGAreaCollision2D *p_a, const SGAreaCollision2D *p_b) const {
		SGPhysics2DServer *physics_server = SGPhysics2DServer::get_singleton();

		SGCollisionObject2D *a = Object::cast_to<SGCollisionObject2D>((Object *)p_a->get_collider());
		SGCollisionObject2D *b = Object::cast_to<SGCollisionObject2D>((Object *)p_b->get_collider());

		return sg_compare_collision_objects(physics_server->collision_object_get_internal(a->get_rid()), physics_server->collision_object_get_internal(b->get_rid()));
	}
};

void SGAreaCollision2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_collider"), &SGAreaCollision2D::get_collider);
	ClassDB::bind_method(D_METHOD("get_collider_rid"), &SGAreaCollision2D::get_collider_rid);
	ClassDB::bind_method(D_METHOD("get_shape"), &SGAreaCollision2D::get_shape);
	ClassDB::bind_method(D_METHOD("get_shape_rid"), &SGAreaCollision2D::get_shape_rid);


	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider"), "", "get_collider");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider_rid"), "", "get_collider_rid");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape"), "", "get_shape");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape_rid"), "", "get_shape_rid");
}

SGCollisionObject2D *SGAreaCollision2D::get_collider() const {
	return collider;
}

SGFixedNode2D *SGAreaCollision2D::get_shape() const {
	return shape;
}

RID SGAreaCollision2D::get_collider_rid() const {
	return collider_rid;
}

RID SGAreaCollision2D::get_shape_rid() const {
	return shape_rid;
}

SGAreaCollision2D::SGAreaCollision2D(SGCollisionObject2D *p_object, RID p_object_rid, SGFixedNode2D *p_shape, RID p_shape_rid) {
	collider = p_object;
	collider_rid = p_object_rid;
	shape = p_shape;
	shape_rid = p_shape_rid;
}

SGAreaCollision2D::SGAreaCollision2D() {
	collider = nullptr;
	shape = nullptr;
}

void SGKinematicCollision2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_collider"), &SGKinematicCollision2D::get_collider);
	ClassDB::bind_method(D_METHOD("get_collider_rid"), &SGKinematicCollision2D::get_collider_rid);
	ClassDB::bind_method(D_METHOD("get_normal"), &SGKinematicCollision2D::get_normal);
	ClassDB::bind_method(D_METHOD("get_remainder"), &SGKinematicCollision2D::get_remainder);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider", PROPERTY_HINT_NONE, "", 0), "", "get_collider");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider_rid", PROPERTY_HINT_NONE, "", 0), "", "get_collider_rid");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal", PROPERTY_HINT_NONE, "", 0), "", "get_normal");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "remainder", PROPERTY_HINT_NONE, "", 0), "", "get_remainder");
}

Object *SGKinematicCollision2D::get_collider() const {
	return collider;
}

RID SGKinematicCollision2D::get_collider_rid() const {
	return collider_rid;
}

Ref<SGFixedVector2> SGKinematicCollision2D::get_normal() const {
	return normal;
}

Ref<SGFixedVector2> SGKinematicCollision2D::get_remainder() const {
	return remainder;
}

SGKinematicCollision2D::SGKinematicCollision2D(SGCollisionObject2D *p_collider, RID p_collider_rid, const Ref<SGFixedVector2> &p_normal, const Ref<SGFixedVector2> &p_remainder) {
	collider = p_collider;
	collider_rid = p_collider_rid;
	normal = p_normal;
	remainder = p_remainder;
}

SGKinematicCollision2D::SGKinematicCollision2D() {
	collider = nullptr;
}

void SGRayCastCollision2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_collider"), &SGRayCastCollision2D::get_collider);
	ClassDB::bind_method(D_METHOD("get_collider_rid"), &SGRayCastCollision2D::get_collider_rid);
	ClassDB::bind_method(D_METHOD("get_point"), &SGRayCastCollision2D::get_point);
	ClassDB::bind_method(D_METHOD("get_normal"), &SGRayCastCollision2D::get_normal);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider", PROPERTY_HINT_NONE, "", 0), "", "get_collider");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collider_rid", PROPERTY_HINT_NONE, "", 0), "", "get_collider_rid");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "point", PROPERTY_HINT_NONE, "", 0), "", "get_point");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal", PROPERTY_HINT_NONE, "", 0), "", "get_normal");
}

Object *SGRayCastCollision2D::get_collider() const {
	return collider;
}

RID SGRayCastCollision2D::get_collider_rid() const {
	return collider_rid;
}

Ref<SGFixedVector2> SGRayCastCollision2D::get_point() const {
	return point;
}

Ref<SGFixedVector2> SGRayCastCollision2D::get_normal() const {
	return normal;
}

SGRayCastCollision2D::SGRayCastCollision2D(SGCollisionObject2D *p_collider, RID p_collider_rid, const Ref<SGFixedVector2> &p_point, const Ref<SGFixedVector2> &p_normal) {
	collider = p_collider;
	collider_rid = p_collider_rid;
	point = p_point;
	normal = p_normal;
}

SGRayCastCollision2D::SGRayCastCollision2D() {
	collider = nullptr;
}

SGPhysics2DServer::ShapeData::~ShapeData() {
	memdelete(internal);
}

SGPhysics2DServer::ObjectData::~ObjectData() {
	memdelete(internal);
}

SGPhysics2DServer::WorldData::~WorldData() {
	memdelete(internal);
}

void SGPhysics2DServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("shape_create", "shape_type"), &SGPhysics2DServer::shape_create);
	ClassDB::bind_method(D_METHOD("shape_get_type", "shape"), &SGPhysics2DServer::shape_get_type);
	ClassDB::bind_method(D_METHOD("shape_set_data", "shape", "data"), &SGPhysics2DServer::shape_set_data);
	ClassDB::bind_method(D_METHOD("shape_get_data", "shape"), &SGPhysics2DServer::shape_get_data);
	ClassDB::bind_method(D_METHOD("shape_set_transform", "shape", "transform"), &SGPhysics2DServer::shape_set_transform);
	ClassDB::bind_method(D_METHOD("shape_get_transform", "shape"), &SGPhysics2DServer::shape_get_transform);

	ClassDB::bind_method(D_METHOD("rectangle_set_extents", "shape", "extents"), &SGPhysics2DServer::rectangle_set_extents);
	ClassDB::bind_method(D_METHOD("rectangle_get_extents", "shape"), &SGPhysics2DServer::rectangle_get_extents);

	ClassDB::bind_method(D_METHOD("circle_set_radius", "shape", "radius"), &SGPhysics2DServer::circle_set_radius);
	ClassDB::bind_method(D_METHOD("circle_get_radius", "shape"), &SGPhysics2DServer::circle_get_radius);

	ClassDB::bind_method(D_METHOD("polygon_set_points", "shape", "points"), &SGPhysics2DServer::polygon_set_points);
	ClassDB::bind_method(D_METHOD("polygon_get_points", "shape"), &SGPhysics2DServer::polygon_get_points);

	ClassDB::bind_method(D_METHOD("collision_object_create", "object_type", "body_type"), &SGPhysics2DServer::collision_object_create, DEFVAL(BODY_UNKNOWN));
	ClassDB::bind_method(D_METHOD("collision_object_get_type", "object"), &SGPhysics2DServer::collision_object_get_type);
	ClassDB::bind_method(D_METHOD("collision_object_set_data", "object", "data"), &SGPhysics2DServer::collision_object_set_data);
	ClassDB::bind_method(D_METHOD("collision_object_get_data", "object"), &SGPhysics2DServer::collision_object_get_data);
	ClassDB::bind_method(D_METHOD("collision_object_set_transform", "object", "transform"), &SGPhysics2DServer::collision_object_set_transform);
	ClassDB::bind_method(D_METHOD("collision_object_get_transform", "object"), &SGPhysics2DServer::collision_object_get_transform);
	ClassDB::bind_method(D_METHOD("collision_object_add_shape", "object", "shape"), &SGPhysics2DServer::collision_object_add_shape);
	ClassDB::bind_method(D_METHOD("collision_object_remove_shape", "object", "shape"), &SGPhysics2DServer::collision_object_remove_shape);
	ClassDB::bind_method(D_METHOD("collision_object_set_collision_layer", "object", "layer"), &SGPhysics2DServer::collision_object_set_collision_layer);
	ClassDB::bind_method(D_METHOD("collision_object_get_collision_layer", "object"), &SGPhysics2DServer::collision_object_get_collision_layer);
	ClassDB::bind_method(D_METHOD("collision_object_set_collision_mask", "object", "mask"), &SGPhysics2DServer::collision_object_set_collision_mask);
	ClassDB::bind_method(D_METHOD("collision_object_get_collision_mask", "object"), &SGPhysics2DServer::collision_object_get_collision_mask);
	ClassDB::bind_method(D_METHOD("collision_object_set_monitorable", "object", "monitorable"), &SGPhysics2DServer::collision_object_set_monitorable);
	ClassDB::bind_method(D_METHOD("collision_object_get_monitorable", "object"), &SGPhysics2DServer::collision_object_get_monitorable);

	ClassDB::bind_method(D_METHOD("area_get_overlapping_areas", "area"), &SGPhysics2DServer::area_get_overlapping_areas);
	ClassDB::bind_method(D_METHOD("area_get_overlapping_bodies", "area"), &SGPhysics2DServer::area_get_overlapping_bodies);
	ClassDB::bind_method(D_METHOD("area_get_overlapping_area_collisions", "area"), &SGPhysics2DServer::area_get_overlapping_area_collisions);
	ClassDB::bind_method(D_METHOD("area_get_overlapping_body_collisions", "area"), &SGPhysics2DServer::area_get_overlapping_body_collisions);
	ClassDB::bind_method(D_METHOD("area_get_overlapping_area_count", "area"), &SGPhysics2DServer::area_get_overlapping_area_count);
	ClassDB::bind_method(D_METHOD("area_get_overlapping_body_count", "area"), &SGPhysics2DServer::area_get_overlapping_body_count);

	ClassDB::bind_method(D_METHOD("body_get_type", "body"), &SGPhysics2DServer::body_get_type);
	ClassDB::bind_method(D_METHOD("body_set_safe_margin", "body", "safe_margin"), &SGPhysics2DServer::body_set_safe_margin);
	ClassDB::bind_method(D_METHOD("body_get_safe_margin", "body"), &SGPhysics2DServer::body_get_safe_margin);
	ClassDB::bind_method(D_METHOD("body_unstuck", "body", "max_attempts"), &SGPhysics2DServer::body_unstuck);
	ClassDB::bind_method(D_METHOD("body_move_and_collide", "body", "linear_velocity"), &SGPhysics2DServer::body_move_and_collide);

	ClassDB::bind_method(D_METHOD("world_create"), &SGPhysics2DServer::world_create);
	ClassDB::bind_method(D_METHOD("get_default_world"), &SGPhysics2DServer::get_default_world);
	ClassDB::bind_method(D_METHOD("world_add_collision_object", "world", "object"), &SGPhysics2DServer::world_add_collision_object);
	ClassDB::bind_method(D_METHOD("world_remove_collision_object", "world", "object"), &SGPhysics2DServer::world_remove_collision_object);

	ClassDB::bind_method(D_METHOD("world_cast_ray", "world", "start", "cast_to", "collision_mask", "exceptions", "collide_with_areas", "collide_with_bodies"), &SGPhysics2DServer::world_cast_ray, DEFVAL(Array()), DEFVAL(false), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &SGPhysics2DServer::free_rid);

	BIND_ENUM_CONSTANT(SHAPE_UNKNOWN);
	BIND_ENUM_CONSTANT(SHAPE_RECTANGLE);
	BIND_ENUM_CONSTANT(SHAPE_CIRCLE);
	BIND_ENUM_CONSTANT(SHAPE_POLYGON);
	BIND_ENUM_CONSTANT(SHAPE_CAPSULE);

	BIND_ENUM_CONSTANT(OBJECT_UNKNOWN);
	BIND_ENUM_CONSTANT(OBJECT_AREA);
	BIND_ENUM_CONSTANT(OBJECT_BODY);
	BIND_ENUM_CONSTANT(OBJECT_BOTH);

	BIND_ENUM_CONSTANT(BODY_UNKNOWN);
	BIND_ENUM_CONSTANT(BODY_STATIC);
	BIND_ENUM_CONSTANT(BODY_KINEMATIC);
}

SGPhysics2DServer *SGPhysics2DServer::get_singleton() {
	return singleton;
}

RID SGPhysics2DServer::shape_create(SGPhysics2DServer::ShapeType p_shape_type) {
	ERR_FAIL_COND_V(p_shape_type == SHAPE_UNKNOWN, RID());
	ShapeData *shape = nullptr;
	switch (p_shape_type) {
		case SHAPE_RECTANGLE: {
			shape = memnew(ShapeData(memnew(SGRectangle2DInternal(SGFixedVector2Internal(fixed(655360), fixed(655360))))));
		} break;
		case SHAPE_CIRCLE: {
			shape = memnew(ShapeData(memnew(SGCircle2DInternal(fixed(655360)))));
		}
		break;
		case SHAPE_CAPSULE: {
			shape = memnew(ShapeData(memnew(SGCapsule2DInternal(fixed(655360), fixed(655360)))));
		}
		break;
		case SHAPE_POLYGON: {
			shape = memnew(ShapeData(memnew(SGPolygon2DInternal)));
		}
		break;
		default:
			ERR_FAIL_V_MSG(RID(), "Unknown shape type");
		}
	RID id = shape_owner.make_rid(shape);
	shape->get_internal()->set_data(memnew(SGInternalData(id)));
	return id;
}

SGPhysics2DServer::ShapeType SGPhysics2DServer::shape_get_type(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, SHAPE_UNKNOWN);
	return (SGPhysics2DServer::ShapeType)data->get_internal()->get_shape_type();
}

void SGPhysics2DServer::shape_set_data(RID p_shape, const Variant &p_data) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	((SGInternalData *)data->get_internal()->get_data())->data = p_data;
}

Variant SGPhysics2DServer::shape_get_data(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, Variant());
	return ((SGInternalData *)data->get_internal()->get_data())->data;
}

void SGPhysics2DServer::shape_set_transform(RID p_shape, const Ref<SGFixedTransform2D> &p_transform) {
	ERR_FAIL_COND(p_transform.is_null());
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	data->get_internal()->set_transform(p_transform->get_internal());
}

Ref<SGFixedTransform2D> SGPhysics2DServer::shape_get_transform(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, Ref<SGFixedTransform2D>());
	return SGFixedTransform2D::from_internal(data->get_internal()->get_transform());
}

void SGPhysics2DServer::rectangle_set_extents(RID p_shape, const Ref<SGFixedVector2> p_extents) {
	ERR_FAIL_COND(p_extents.is_null());
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_RECTANGLE);
	((SGRectangle2DInternal *)data->get_internal())->set_extents(p_extents->get_internal());
}

Ref<SGFixedVector2> SGPhysics2DServer::rectangle_get_extents(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, Ref<SGFixedVector2>());
	ERR_FAIL_COND_V(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_RECTANGLE, Ref<SGFixedVector2>());
	return SGFixedVector2::from_internal(((SGRectangle2DInternal *)data->get_internal())->get_extents());
}

void SGPhysics2DServer::circle_set_radius(RID p_shape, int64_t p_radius) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CIRCLE);
	((SGCircle2DInternal *)data->get_internal())->set_radius(fixed(p_radius));
}

int64_t SGPhysics2DServer::circle_get_radius(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CIRCLE, 0);
	return ((SGCircle2DInternal *)data->get_internal())->get_radius().value;
}

void SGPhysics2DServer::capsule_set_radius(RID p_shape, int64_t p_radius) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CAPSULE);
	((SGCapsule2DInternal *)data->get_internal())->set_radius(fixed(p_radius));
}

int64_t SGPhysics2DServer::capsule_get_radius(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CAPSULE, 0);
	return ((SGCapsule2DInternal *)data->get_internal())->get_radius().value;
}

void SGPhysics2DServer::capsule_set_height(RID p_shape, int64_t p_height) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CAPSULE);
	((SGCapsule2DInternal *)data->get_internal())->set_height(fixed(p_height));
}

int64_t SGPhysics2DServer::capsule_get_height(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_CAPSULE, 0);
	return ((SGCapsule2DInternal *)data->get_internal())->get_height().value;
}

void SGPhysics2DServer::polygon_set_points(RID p_shape, const Array &p_points_array) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_POLYGON);

	std::vector<SGFixedVector2Internal> points;
	points.resize(p_points_array.size());

	for (int i = 0; i < p_points_array.size(); i++) {
		Ref<SGFixedVector2> point = p_points_array[i];
		if (point.is_valid()) {
			points[i] = point->get_internal();
		}
	}

	((SGPolygon2DInternal *)data->get_internal())->set_points(points);
}

Array SGPhysics2DServer::polygon_get_points(RID p_shape) const {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, Array());
	ERR_FAIL_COND_V(data->get_internal()->get_shape_type() != SGShape2DInternal::SHAPE_POLYGON, Array());

	Array ret;
	std::vector<SGFixedVector2Internal> points = ((SGPolygon2DInternal *)data->get_internal())->get_points();
	ret.resize(points.size());
	for (std::size_t i = 0; i < points.size(); i++) {
		ret.set(i, SGFixedVector2::from_internal(points[i]));
	}
	return ret;
}

RID SGPhysics2DServer::collision_object_create(SGPhysics2DServer::CollisionObjectType p_object_type, SGPhysics2DServer::BodyType p_body_type) {
	ERR_FAIL_COND_V(p_object_type == OBJECT_BOTH, RID());
	ERR_FAIL_COND_V(p_object_type == OBJECT_BODY && p_body_type == BODY_UNKNOWN, RID());
	ObjectData *data = nullptr;
	switch (p_object_type) {
		case OBJECT_AREA: {
			data = memnew(ObjectData(memnew(SGArea2DInternal())));
		} break;
		case OBJECT_BODY: {
			data = memnew(ObjectData(memnew(SGBody2DInternal((SGBody2DInternal::BodyType)p_body_type))));
		} break;
		default:
			ERR_FAIL_V_MSG(RID(), "Unknown collision object type");
	}
	RID id = object_owner.make_rid(data);
	data->get_internal()->set_data(memnew(SGInternalData(id)));
	return id;
}

SGPhysics2DServer::CollisionObjectType SGPhysics2DServer::collision_object_get_type(RID p_object) {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, OBJECT_UNKNOWN);
	return (SGPhysics2DServer::CollisionObjectType)data->get_internal()->get_object_type();
}

void SGPhysics2DServer::collision_object_set_data(RID p_object, const Variant &p_data) {
	Variant::Type t = p_data.get_type();
	ERR_FAIL_COND_MSG(!(t == Variant::OBJECT && Object::cast_to<Node>(p_data) != nullptr) && t != Variant::STRING && t != Variant::NIL, "Data can only be Node, string or null");

	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND(!data);
	((SGInternalData *)data->get_internal()->get_data())->data = p_data;
}

Variant SGPhysics2DServer::collision_object_get_data(RID p_object) const {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, Variant());
	return ((SGInternalData *)data->get_internal()->get_data())->data;
}

void SGPhysics2DServer::collision_object_set_transform(RID p_object, const Ref<SGFixedTransform2D> &p_transform) {
	ERR_FAIL_COND(p_transform.is_null());
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND(!data);
	data->get_internal()->set_transform(p_transform->get_internal());
}

Ref<SGFixedTransform2D> SGPhysics2DServer::collision_object_get_transform(RID p_object) const {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, Ref<SGFixedTransform2D>());
	return SGFixedTransform2D::from_internal(data->get_internal()->get_transform());
}

void SGPhysics2DServer::collision_object_add_shape(RID p_object, RID p_shape) {
	ObjectData *object_data = object_owner.get(p_object);
	ERR_FAIL_COND(!object_data);
	ShapeData *shape_data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!shape_data);

	object_data->get_internal()->add_shape(shape_data->get_internal());
}

void SGPhysics2DServer::collision_object_remove_shape(RID p_object, RID p_shape) {
	ObjectData *object_data = object_owner.get(p_object);
	ERR_FAIL_COND(!object_data);
	ShapeData *shape_data = shape_owner.get(p_shape);
	ERR_FAIL_COND(!shape_data);

	object_data->get_internal()->remove_shape(shape_data->get_internal());
}

void SGPhysics2DServer::collision_object_set_collision_layer(RID p_object, uint32_t p_collision_layer) {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND(!data);
	data->get_internal()->set_collision_layer(p_collision_layer);
}

uint32_t SGPhysics2DServer::collision_object_get_collision_layer(RID p_object) const {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, 0);
	return data->get_internal()->get_collision_layer();
}

void SGPhysics2DServer::collision_object_set_collision_mask(RID p_object, uint32_t p_collision_mask) {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND(!data);
	data->get_internal()->set_collision_mask(p_collision_mask);
}

uint32_t SGPhysics2DServer::collision_object_get_collision_mask(RID p_object) const {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, 0);
	return data->get_internal()->get_collision_mask();
}

void SGPhysics2DServer::collision_object_set_monitorable(RID p_object, bool p_monitorable) {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND(!data);
	data->get_internal()->set_monitorable(p_monitorable);
}

bool SGPhysics2DServer::collision_object_get_monitorable(RID p_object) const {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, false);
	return data->get_internal()->get_monitorable();
}

class SGSortedArrayResultHandler : public SGResultHandlerInternal {
private:

	List<SGCollisionObject2DInternal *> result;

public:
	void handle_result(SGCollisionObject2DInternal *p_object, SGShape2DInternal *p_shape) {
		result.push_back(p_object);
	}

	_FORCE_INLINE_ Array get_array() {
		if (result.size() > 1) {
			result.sort_custom<SGCollisionObjectComparator>();
		}

		Array ret;
		for (List<SGCollisionObject2DInternal *>::Element *E = result.front(); E; E = E->next()) {
			SGInternalData *object_data = (SGInternalData *)E->get()->get_data();
			SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>(object_data->get_object());
			if (object) {
				ret.push_back(object);
			} else {
				ret.push_back(object_data->rid);
			}
		}

		return ret;
	}
};

Array SGPhysics2DServer::area_get_overlapping_areas(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, Array());
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, Array());

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), Array());

	SGSortedArrayResultHandler result_handler;
	internal->get_world()->get_overlapping_areas(internal, &result_handler);
	return result_handler.get_array();
}

Array SGPhysics2DServer::area_get_overlapping_bodies(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, Array());
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, Array());

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), Array());

	SGSortedArrayResultHandler result_handler;
	internal->get_world()->get_overlapping_bodies(internal, &result_handler);
	return result_handler.get_array();
}

class SGSortedCollisionArrayResultHandler : public SGResultHandlerInternal {
private:

	List<SGAreaCollision2D *> result;

public:
	void handle_result(SGCollisionObject2DInternal *p_object, SGShape2DInternal *p_shape) {
		SGInternalData *object_data = (SGInternalData *)p_object->get_data();
		SGInternalData *shape_data = (SGInternalData *)p_shape->get_data();
		SGAreaCollision2D *collision = memnew(SGAreaCollision2D(
			Object::cast_to<SGCollisionObject2D>(object_data->get_object()),
			object_data->rid,
			Object::cast_to<SGFixedNode2D>(shape_data->get_object()),
			shape_data->rid
		));
		result.push_back(collision);
	}

	_FORCE_INLINE_ Array get_array() {
		if (result.size() > 1) {
			result.sort_custom<SGAreaCollision2DComparator>();
		}

		Array ret;
		for (List<SGAreaCollision2D *>::Element *E = result.front(); E; E = E->next()) {
			ret.push_back(E->get());
		}

		return ret;
	}
};

Array SGPhysics2DServer::area_get_overlapping_area_collisions(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, Array());
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, Array());

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), Array());

	SGSortedCollisionArrayResultHandler result_handler;
	internal->get_world()->get_overlapping_areas(internal, &result_handler);
	return result_handler.get_array();
}

Array SGPhysics2DServer::area_get_overlapping_body_collisions(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, Array());
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, Array());

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), Array());

	SGSortedCollisionArrayResultHandler result_handler;
	internal->get_world()->get_overlapping_bodies(internal, &result_handler);
	return result_handler.get_array();
}

class SGCountResultHandler : public SGResultHandlerInternal {
private:

	int count = 0;

public:
	void handle_result(SGCollisionObject2DInternal *p_object, SGShape2DInternal *p_shape) {
		count++;
	}

	_FORCE_INLINE_ int get_count() {
		return count;
	}
};

int SGPhysics2DServer::area_get_overlapping_area_count(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, 0);

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), 0);

	SGCountResultHandler result_handler;
	internal->get_world()->get_overlapping_areas(internal, &result_handler);
	return result_handler.get_count();
}

int SGPhysics2DServer::area_get_overlapping_body_count(RID p_area) const {
	ObjectData *data = object_owner.get(p_area);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_AREA, 0);

	SGArea2DInternal *internal = ((SGArea2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), 0);

	SGCountResultHandler result_handler;
	internal->get_world()->get_overlapping_bodies(internal, &result_handler);
	return result_handler.get_count();
}

SGPhysics2DServer::BodyType SGPhysics2DServer::body_get_type(RID p_body) const {
	ObjectData *data = object_owner.get(p_body);
	ERR_FAIL_COND_V(!data, BODY_UNKNOWN);
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_BODY, BODY_UNKNOWN);
	return (SGPhysics2DServer::BodyType)((SGBody2DInternal *)data->get_internal())->get_body_type();
}

void SGPhysics2DServer::body_set_safe_margin(RID p_body, int p_safe_margin) {
	ObjectData *data = object_owner.get(p_body);
	ERR_FAIL_COND(!data);
	ERR_FAIL_COND(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_BODY);
	((SGBody2DInternal *)data->get_internal())->set_safe_margin(fixed(p_safe_margin));
}

int SGPhysics2DServer::body_get_safe_margin(RID p_body) const {
	ObjectData *data = object_owner.get(p_body);
	ERR_FAIL_COND_V(!data, 0);
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_BODY, 0);
	return ((SGBody2DInternal *)data->get_internal())->get_safe_margin().value;
}

bool SGPhysics2DServer::body_unstuck(RID p_body, int p_max_attempts) const {
	ObjectData *data = object_owner.get(p_body);
	ERR_FAIL_COND_V(!data, false);
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_BODY, false);

	SGBody2DInternal *internal = ((SGBody2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), false);

	return internal->get_world()->unstuck_body(internal, p_max_attempts);
}

Ref<SGKinematicCollision2D> SGPhysics2DServer::body_move_and_collide(RID p_body, const Ref<SGFixedVector2> &p_linear_velocity) const {
	ObjectData *data = object_owner.get(p_body);
	ERR_FAIL_COND_V(!data, Ref<SGKinematicCollision2D>());
	ERR_FAIL_COND_V(data->get_internal()->get_object_type() != SGCollisionObject2DInternal::OBJECT_BODY, Ref<SGKinematicCollision2D>());

	SGBody2DInternal *internal = ((SGBody2DInternal *)data->get_internal());
	ERR_FAIL_COND_V(!internal->get_world(), Ref<SGKinematicCollision2D>());

	SGWorld2DInternal::BodyCollisionInfo collision;
	if (internal->get_world()->move_and_collide(internal, p_linear_velocity->get_internal(), &collision)) {
		SGInternalData *object_data = (SGInternalData *)collision.collider->get_data();
		SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>(object_data->get_object());
		Ref<SGKinematicCollision2D> result = Ref<SGKinematicCollision2D>(memnew(SGKinematicCollision2D(
			object,
			object_data->rid,
			SGFixedVector2::from_internal(collision.normal),
			SGFixedVector2::from_internal(collision.remainder)
		)));
		return result;
	}

	return Ref<SGKinematicCollision2D>();
}

RID SGPhysics2DServer::world_create() {
	int cell_size = 128;
	if (ProjectSettings::get_singleton()->has_setting("physics/2d/cell_size")) {
		cell_size = ProjectSettings::get_singleton()->get_setting("physics/2d/cell_size");
	}

	WorldData *data = memnew(WorldData(memnew(SGWorld2DInternal(cell_size, &sg_compare_collision_objects))));
	return world_owner.make_rid(data);
}

RID SGPhysics2DServer::get_default_world() {
	return default_world;
}

void SGPhysics2DServer::world_add_collision_object(RID p_world, RID p_object) {
	WorldData *world_data = world_owner.get(p_world);
	ERR_FAIL_COND(!world_data);

	ObjectData *object_data = object_owner.get(p_object);
	ERR_FAIL_COND(!object_data);

	SGCollisionObject2DInternal *object_internal = object_data->get_internal();
	if (object_internal->get_object_type() == SGCollisionObject2DInternal::OBJECT_BODY) {
		world_data->get_internal()->add_body((SGBody2DInternal *)object_internal);
	}
	else if (object_internal->get_object_type() == SGCollisionObject2DInternal::OBJECT_AREA) {
		world_data->get_internal()->add_area((SGArea2DInternal *)object_internal);
	}
}

void SGPhysics2DServer::world_remove_collision_object(RID p_world, RID p_object) {
	WorldData *world_data = world_owner.get(p_world);
	ERR_FAIL_COND(!world_data);

	ObjectData *object_data = object_owner.get(p_object);
	ERR_FAIL_COND(!object_data);

	SGCollisionObject2DInternal *object_internal = object_data->get_internal();
	if (object_internal->get_object_type() == SGCollisionObject2DInternal::OBJECT_BODY) {
		world_data->get_internal()->remove_body((SGBody2DInternal *)object_internal);
	}
	else if (object_internal->get_object_type() == SGCollisionObject2DInternal::OBJECT_AREA) {
		world_data->get_internal()->remove_area((SGArea2DInternal *)object_internal);
	}
}

Ref<SGRayCastCollision2D> SGPhysics2DServer::world_cast_ray(RID p_world, const Ref<SGFixedVector2> &p_start, const Ref<SGFixedVector2> &p_cast_to, uint32_t p_collision_mask, Array p_exceptions, bool p_collide_with_areas, bool p_collide_with_bodies) {
	WorldData *world_data = world_owner.get(p_world);
	ERR_FAIL_COND_V(!world_data, Ref<SGRayCastCollision2D>());

	std::unordered_set<SGCollisionObject2DInternal *> exceptions;
	for (int i = 0; i < p_exceptions.size(); i++) {
		SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>(p_exceptions[i]);
		if (object) {
			exceptions.insert(collision_object_get_internal(object->get_rid()));
		}
	}

	SGWorld2DInternal::RayCastInfo info;
	Ref<SGRayCastCollision2D> ret;

	if (world_data->get_internal()->cast_ray(p_start->get_internal(), p_cast_to->get_internal(), p_collision_mask, &exceptions, p_collide_with_areas, p_collide_with_bodies, &info)) {
		SGInternalData *object_data = (SGInternalData *)info.body->get_data();
		SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>(object_data->get_object());
		ret = Ref<SGRayCastCollision2D>(memnew(SGRayCastCollision2D(
				object,
				object_data->rid,
				SGFixedVector2::from_internal(info.collision_point),
				SGFixedVector2::from_internal(info.collision_normal))));
	}

	return ret;
}

void SGPhysics2DServer::free_rid(RID p_rid) {
	if (shape_owner.owns(p_rid)) {
		ShapeData *shape_data = shape_owner.get(p_rid);
		SGShape2DInternal *shape = shape_data->get_internal();
		SGCollisionObject2DInternal *owner = shape->get_owner();
		if (owner) {
			owner->remove_shape(shape);
		}
		shape_owner.free(p_rid);
		memdelete((SGInternalData *)shape_data->get_internal()->get_data());
		memdelete(shape_data);
	} else if (object_owner.owns(p_rid)) {
		ObjectData *object_data = object_owner.get(p_rid);
		SGCollisionObject2DInternal *object = object_data->get_internal();
		SGWorld2DInternal *world = object->get_world();
		if (world) {
			SGCollisionObject2DInternal::ObjectType object_type = object->get_object_type();
			if (object_type == SGCollisionObject2DInternal::OBJECT_AREA) {
				world->remove_area((SGArea2DInternal *)object);
			}
			else if (object_type == SGCollisionObject2DInternal::OBJECT_BODY) {
				world->remove_body((SGBody2DInternal *)object);
			}
		}
		object_owner.free(p_rid);
		memdelete((SGInternalData *)object_data->get_internal()->get_data());
		memdelete(object_data);
	} else if (world_owner.owns(p_rid)) {
		ERR_FAIL_COND_MSG(p_rid == default_world, "Cannot free the default world");
		WorldData *world_data = world_owner.get(p_rid);
		world_owner.free(p_rid);
		memdelete(world_data);
	}
}

SGShape2DInternal *SGPhysics2DServer::shape_get_internal(RID p_shape) {
	ShapeData *data = shape_owner.get(p_shape);
	ERR_FAIL_COND_V(!data, nullptr);
	return data->get_internal();
}

SGCollisionObject2DInternal *SGPhysics2DServer::collision_object_get_internal(RID p_object) {
	ObjectData *data = object_owner.get(p_object);
	ERR_FAIL_COND_V(!data, nullptr);
	return data->get_internal();
}

SGWorld2DInternal *SGPhysics2DServer::world_get_internal(RID p_world) {
	WorldData *data = world_owner.get(p_world);
	ERR_FAIL_COND_V(!data, nullptr);
	return data->get_internal();
}

SGWorld2DInternal *SGPhysics2DServer::get_default_world_internal() {
	return world_get_internal(default_world);
}

SGPhysics2DServer::SGPhysics2DServer() {
	singleton = this;
	default_world = world_create();
}

SGPhysics2DServer::~SGPhysics2DServer() {
	singleton = nullptr;

	WorldData *default_world_data = world_owner.get(default_world);
	world_owner.free(default_world);
	memdelete(default_world_data);
}
