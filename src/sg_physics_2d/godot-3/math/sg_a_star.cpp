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

// Code originally from Godot Engine's AStar (MIT License)

#include "sg_a_star.h"

#include "core/math/geometry.h"
#include "core/script_language.h"
#include "scene/scene_string_names.h"

int SGAStar2D::get_available_point_id() const {
	if (points.has(last_free_id)) {
		int cur_new_id = last_free_id + 1;
		while (points.has(cur_new_id)) {
			cur_new_id++;
		}
		const_cast<int &>(last_free_id) = cur_new_id;
	}

	return last_free_id;
}

void SGAStar2D::add_point(int p_id, const Ref<SGFixedVector2> &p_pos, int64_t p_weight_scale) {
	ERR_FAIL_COND_MSG(p_id < 0, vformat("Can't add a point with negative id: %d.", p_id));
	ERR_FAIL_COND_MSG(p_weight_scale < fixed::ONE.value, vformat("Can't add a point with weight scale less than one: %d.", p_weight_scale));
	ERR_FAIL_COND_MSG(!p_pos.is_valid(), "The point reference is not valid");

	Point *found_pt;
	bool p_exists = points.lookup(p_id, found_pt);

	if (!p_exists) {
		Point *pt = memnew(Point);
		pt->id = p_id;
		pt->pos = p_pos->get_internal();
		pt->weight_scale = fixed(p_weight_scale);
		pt->prev_point = nullptr;
		pt->open_pass = 0;
		pt->closed_pass = 0;
		pt->enabled = true;
		points.set(p_id, pt);
	} else {
		found_pt->pos = p_pos->get_internal();
		found_pt->weight_scale = fixed(p_weight_scale);
	}
}

Ref<SGFixedVector2> SGAStar2D::get_point_position(int p_id) const {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_V_MSG(!p_exists, SGFixedVector2::from_internal(SGFixedVector2Internal()), vformat("Can't get point's position. Point with id: %d doesn't exist.", p_id));

	return SGFixedVector2::from_internal(p->pos);
}

void SGAStar2D::set_point_position(int p_id, const Ref<SGFixedVector2> &p_pos) {
	ERR_FAIL_COND_MSG(!p_pos.is_valid(), "The point reference is not valid");

	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_MSG(!p_exists, vformat("Can't set point's position. Point with id: %d doesn't exist.", p_id));

	p->pos = p_pos->get_internal();
}

int64_t SGAStar2D::get_point_weight_scale(int p_id) const {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_V_MSG(!p_exists, 0, vformat("Can't get point's weight scale. Point with id: %d doesn't exist.", p_id));

	return p->weight_scale.value;
}

void SGAStar2D::set_point_weight_scale(int p_id, int64_t p_weight_scale) {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_MSG(!p_exists, vformat("Can't set point's weight scale. Point with id: %d doesn't exist.", p_id));
	ERR_FAIL_COND_MSG(p_weight_scale < fixed::ONE.value, vformat("Can't set point's weight scale less than one: %d.", p_weight_scale));

	p->weight_scale = fixed(p_weight_scale);
}

void SGAStar2D::remove_point(int p_id) {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_MSG(!p_exists, vformat("Can't remove point. Point with id: %d doesn't exist.", p_id));

	for (OAHashMap<int, Point *>::Iterator it = p->neighbours.iter(); it.valid; it = p->neighbours.next_iter(it)) {
		Segment s(p_id, (*it.key));
		segments.erase(s);

		(*it.value)->neighbours.remove(p->id);
		(*it.value)->unlinked_neighbours.remove(p->id);
	}

	for (OAHashMap<int, Point *>::Iterator it = p->unlinked_neighbours.iter(); it.valid; it = p->unlinked_neighbours.next_iter(it)) {
		Segment s(p_id, (*it.key));
		segments.erase(s);

		(*it.value)->neighbours.remove(p->id);
		(*it.value)->unlinked_neighbours.remove(p->id);
	}

	memdelete(p);
	points.remove(p_id);
	last_free_id = p_id;
}

void SGAStar2D::connect_points(int p_id, int p_with_id, bool bidirectional) {
	ERR_FAIL_COND_MSG(p_id == p_with_id, vformat("Can't connect point with id: %d to itself.", p_id));

	Point *a;
	bool from_exists = points.lookup(p_id, a);
	ERR_FAIL_COND_MSG(!from_exists, vformat("Can't connect points. Point with id: %d doesn't exist.", p_id));

	Point *b;
	bool to_exists = points.lookup(p_with_id, b);
	ERR_FAIL_COND_MSG(!to_exists, vformat("Can't connect points. Point with id: %d doesn't exist.", p_with_id));

	a->neighbours.set(b->id, b);

	if (bidirectional) {
		b->neighbours.set(a->id, a);
	} else {
		b->unlinked_neighbours.set(a->id, a);
	}

	Segment s(p_id, p_with_id);
	if (bidirectional) {
		s.direction = Segment::BIDIRECTIONAL;
	}

	Set<Segment>::Element *element = segments.find(s);
	if (element != nullptr) {
		s.direction |= element->get().direction;
		if (s.direction == Segment::BIDIRECTIONAL) {
			// Both are neighbours of each other now
			a->unlinked_neighbours.remove(b->id);
			b->unlinked_neighbours.remove(a->id);
		}
		segments.erase(element);
	}

	segments.insert(s);
}

void SGAStar2D::disconnect_points(int p_id, int p_with_id, bool bidirectional) {
	Point *a;
	bool a_exists = points.lookup(p_id, a);
	ERR_FAIL_COND_MSG(!a_exists, vformat("Can't disconnect points. Point with id: %d doesn't exist.", p_id));

	Point *b;
	bool b_exists = points.lookup(p_with_id, b);
	ERR_FAIL_COND_MSG(!b_exists, vformat("Can't disconnect points. Point with id: %d doesn't exist.", p_with_id));

	Segment s(p_id, p_with_id);
	int remove_direction = bidirectional ? (int)Segment::BIDIRECTIONAL : s.direction;

	Set<Segment>::Element *element = segments.find(s);
	if (element != nullptr) {
		// s is the new segment
		// Erase the directions to be removed
		s.direction = (element->get().direction & ~remove_direction);

		a->neighbours.remove(b->id);
		if (bidirectional) {
			b->neighbours.remove(a->id);
			if (element->get().direction != Segment::BIDIRECTIONAL) {
				a->unlinked_neighbours.remove(b->id);
				b->unlinked_neighbours.remove(a->id);
			}
		} else {
			if (s.direction == Segment::NONE) {
				b->unlinked_neighbours.remove(a->id);
			} else {
				a->unlinked_neighbours.set(b->id, b);
			}
		}

		segments.erase(element);
		if (s.direction != Segment::NONE) {
			segments.insert(s);
		}
	}
}

bool SGAStar2D::has_point(int p_id) const {
	return points.has(p_id);
}

Array SGAStar2D::get_points() {
	Array point_list;

	for (OAHashMap<int, Point *>::Iterator it = points.iter(); it.valid; it = points.next_iter(it)) {
		point_list.push_back(*(it.key));
	}

	return point_list;
}

PoolVector<int> SGAStar2D::get_point_connections(int p_id) {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_V_MSG(!p_exists, PoolVector<int>(), vformat("Can't get point's connections. Point with id: %d doesn't exist.", p_id));

	PoolVector<int> point_list;

	for (OAHashMap<int, Point *>::Iterator it = p->neighbours.iter(); it.valid; it = p->neighbours.next_iter(it)) {
		point_list.push_back((*it.key));
	}

	return point_list;
}

bool SGAStar2D::are_points_connected(int p_id, int p_with_id, bool bidirectional) const {
	Segment s(p_id, p_with_id);
	const Set<Segment>::Element *element = segments.find(s);

	return element != nullptr &&
			(bidirectional || (element->get().direction & s.direction) == s.direction);
}

void SGAStar2D::clear() {
	last_free_id = 0;
	for (OAHashMap<int, Point *>::Iterator it = points.iter(); it.valid; it = points.next_iter(it)) {
		memdelete(*(it.value));
	}
	segments.clear();
	points.clear();
}

int SGAStar2D::get_point_count() const {
	return points.get_num_elements();
}

int SGAStar2D::get_point_capacity() const {
	return points.get_capacity();
}

void SGAStar2D::reserve_space(int p_num_nodes) {
	ERR_FAIL_COND_MSG(p_num_nodes <= 0, vformat("New capacity must be greater than 0, new was: %d.", p_num_nodes));
	ERR_FAIL_COND_MSG((uint32_t)p_num_nodes < points.get_capacity(), vformat("New capacity must be greater than current capacity: %d, new was: %d.", points.get_capacity(), p_num_nodes));
	points.reserve(p_num_nodes);
}

int SGAStar2D::get_closest_point(const Ref<SGFixedVector2> &p_point, bool p_include_disabled) const {
	ERR_FAIL_COND_V_MSG(!p_point.is_valid(), -1, "The point reference is not valid");

    SGFixedVector2Internal p_point_internal = p_point->get_internal();
	int closest_id = -1;
	fixed closest_dist(INT64_MAX);

	for (OAHashMap<int, Point *>::Iterator it = points.iter(); it.valid; it = points.next_iter(it)) {
		if (!p_include_disabled && !(*it.value)->enabled) {
			continue; // Disabled points should not be considered.
		}

		// Keep the closest point's ID, and in case of multiple closest IDs,
		// the smallest one (makes it deterministic).
		fixed d = p_point_internal.distance_to((*it.value)->pos);
		int id = *(it.key);
		if (d <= closest_dist) {
			if (d == closest_dist && id > closest_id) { // Keep lowest ID.
				continue;
			}
			closest_dist = d;
			closest_id = id;
		}
	}

	return closest_id;
}

Ref<SGFixedVector2> SGAStar2D::get_closest_position_in_segment(const Ref<SGFixedVector2> &p_point) const {
	ERR_FAIL_COND_V_MSG(!p_point.is_valid(), SGFixedVector2::from_internal(SGFixedVector2Internal()), "The point reference is not valid");

    SGFixedVector2Internal p_point_internal = p_point->get_internal();
	fixed closest_dist(INT64_MAX);
	SGFixedVector2Internal closest_point;

	for (const Set<Segment>::Element *E = segments.front(); E; E = E->next()) {
		Point *from_point = nullptr, *to_point = nullptr;
		points.lookup(E->get().u, from_point);
		points.lookup(E->get().v, to_point);

		if (!(from_point->enabled && to_point->enabled)) {
			continue;
		}

		SGFixedVector2Internal segment[2] = {
			from_point->pos,
			to_point->pos,
		};

		SGFixedVector2Internal p = SGFixedVector2Internal::get_closest_point_to_segment_2d(p_point_internal, segment);
		fixed d = p_point_internal.distance_to(p);
		if (d < closest_dist) {
			closest_point = p;
			closest_dist = d;
		}
	}

	return SGFixedVector2::from_internal(closest_point);
}

bool SGAStar2D::_solve(Point *begin_point, Point *end_point) {
	pass++;

	if (!end_point->enabled) {
		return false;
	}

	bool found_route = false;

	Vector<Point *> open_list;
	SortArray<Point *, SortPoints> sorter;

	begin_point->g_score = fixed::ZERO;
	begin_point->f_score = fixed(_estimate_cost(begin_point->id, end_point->id));
	open_list.push_back(begin_point);

	while (!open_list.empty()) {
		Point *p = open_list[0]; // The currently processed point

		if (p == end_point) {
			found_route = true;
			break;
		}

		sorter.pop_heap(0, open_list.size(), open_list.ptrw()); // Remove the current point from the open list
		open_list.remove(open_list.size() - 1);
		p->closed_pass = pass; // Mark the point as closed

		for (OAHashMap<int, Point *>::Iterator it = p->neighbours.iter(); it.valid; it = p->neighbours.next_iter(it)) {
			Point *e = *(it.value); // The neighbour point

			if (!e->enabled || e->closed_pass == pass) {
				continue;
			}

			fixed tentative_g_score = p->g_score + fixed(_compute_cost(p->id, e->id)) * e->weight_scale;

			bool new_point = false;

			if (e->open_pass != pass) { // The point wasn't inside the open list.
				e->open_pass = pass;
				open_list.push_back(e);
				new_point = true;
			} else if (tentative_g_score >= e->g_score) { // The new path is worse than the previous.
				continue;
			}

			e->prev_point = p;
			e->g_score = tentative_g_score;
			e->f_score = e->g_score + fixed(_estimate_cost(e->id, end_point->id));

			if (new_point) { // The position of the new points is already known.
				sorter.push_heap(0, open_list.size() - 1, 0, e, open_list.ptrw());
			} else {
				sorter.push_heap(0, open_list.find(e), 0, e, open_list.ptrw());
			}
		}
	}

	return found_route;
}

int64_t SGAStar2D::_estimate_cost(int p_from_id, int p_to_id) {
	if (get_script_instance() && get_script_instance()->has_method(SceneStringNames::get_singleton()->_estimate_cost)) {
		return get_script_instance()->call(SceneStringNames::get_singleton()->_estimate_cost, p_from_id, p_to_id);
	}

	Point *from_point;
	bool from_exists = points.lookup(p_from_id, from_point);
	ERR_FAIL_COND_V_MSG(!from_exists, 0, vformat("Can't estimate cost. Point with id: %d doesn't exist.", p_from_id));

	Point *to_point;
	bool to_exists = points.lookup(p_to_id, to_point);
	ERR_FAIL_COND_V_MSG(!to_exists, 0, vformat("Can't estimate cost. Point with id: %d doesn't exist.", p_to_id));

	return from_point->pos.distance_to(to_point->pos).value;
}

int64_t SGAStar2D::_compute_cost(int p_from_id, int p_to_id) {
	if (get_script_instance() && get_script_instance()->has_method(SceneStringNames::get_singleton()->_compute_cost)) {
		return get_script_instance()->call(SceneStringNames::get_singleton()->_compute_cost, p_from_id, p_to_id);
	}

	Point *from_point;
	bool from_exists = points.lookup(p_from_id, from_point);
	ERR_FAIL_COND_V_MSG(!from_exists, 0, vformat("Can't compute cost. Point with id: %d doesn't exist.", p_from_id));

	Point *to_point;
	bool to_exists = points.lookup(p_to_id, to_point);
	ERR_FAIL_COND_V_MSG(!to_exists, 0, vformat("Can't compute cost. Point with id: %d doesn't exist.", p_to_id));

	return from_point->pos.distance_to(to_point->pos).value;
}

Array SGAStar2D::get_point_path(int p_from_id, int p_to_id) {
	Point *a;
	bool from_exists = points.lookup(p_from_id, a);
	ERR_FAIL_COND_V_MSG(!from_exists, Array(), vformat("Can't get point path. Point with id: %d doesn't exist.", p_from_id));

	Point *b;
	bool to_exists = points.lookup(p_to_id, b);
	ERR_FAIL_COND_V_MSG(!to_exists, Array(), vformat("Can't get point path. Point with id: %d doesn't exist.", p_to_id));

	if (a == b) {
		Array ret;
		ret.push_back(SGFixedVector2::from_internal(a->pos));
		return ret;
	}

	Point *begin_point = a;
	Point *end_point = b;

	bool found_route = _solve(begin_point, end_point);
	if (!found_route) {
		return Array();
	}

	Point *p = end_point;
	int pc = 1; // Begin point
	while (p != begin_point) {
		pc++;
		p = p->prev_point;
	}

	Array path;
	path.resize(pc);

	{
		Point *p2 = end_point;
		int idx = pc - 1;
		while (p2 != begin_point) {
			path[idx--] = SGFixedVector2::from_internal(p2->pos);
			p2 = p2->prev_point;
		}

		path[0] = SGFixedVector2::from_internal(p2->pos); // Assign first
	}

	return path;
}

PoolVector<int> SGAStar2D::get_id_path(int p_from_id, int p_to_id) {
	Point *a;
	bool from_exists = points.lookup(p_from_id, a);
	ERR_FAIL_COND_V_MSG(!from_exists, PoolVector<int>(), vformat("Can't get id path. Point with id: %d doesn't exist.", p_from_id));

	Point *b;
	bool to_exists = points.lookup(p_to_id, b);
	ERR_FAIL_COND_V_MSG(!to_exists, PoolVector<int>(), vformat("Can't get id path. Point with id: %d doesn't exist.", p_to_id));

	if (a == b) {
		PoolVector<int> ret;
		ret.push_back(a->id);
		return ret;
	}

	Point *begin_point = a;
	Point *end_point = b;

	bool found_route = _solve(begin_point, end_point);
	if (!found_route) {
		return PoolVector<int>();
	}

	Point *p = end_point;
	int pc = 1; // Begin point
	while (p != begin_point) {
		pc++;
		p = p->prev_point;
	}

	PoolVector<int> path;
	path.resize(pc);

	{
		PoolVector<int>::Write w = path.write();

		p = end_point;
		int idx = pc - 1;
		while (p != begin_point) {
			w[idx--] = p->id;
			p = p->prev_point;
		}

		w[0] = p->id; // Assign first
	}

	return path;
}

void SGAStar2D::set_point_disabled(int p_id, bool p_disabled) {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_MSG(!p_exists, vformat("Can't set if point is disabled. Point with id: %d doesn't exist.", p_id));

	p->enabled = !p_disabled;
}

bool SGAStar2D::is_point_disabled(int p_id) const {
	Point *p;
	bool p_exists = points.lookup(p_id, p);
	ERR_FAIL_COND_V_MSG(!p_exists, false, vformat("Can't get if point is disabled. Point with id: %d doesn't exist.", p_id));

	return !p->enabled;
}

void SGAStar2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_available_point_id"), &SGAStar2D::get_available_point_id);
	ClassDB::bind_method(D_METHOD("add_point", "id", "position", "weight_scale"), &SGAStar2D::add_point, DEFVAL(65536));
	ClassDB::bind_method(D_METHOD("get_point_position", "id"), &SGAStar2D::get_point_position);
	ClassDB::bind_method(D_METHOD("set_point_position", "id", "position"), &SGAStar2D::set_point_position);
	ClassDB::bind_method(D_METHOD("get_point_weight_scale", "id"), &SGAStar2D::get_point_weight_scale);
	ClassDB::bind_method(D_METHOD("set_point_weight_scale", "id", "weight_scale"), &SGAStar2D::set_point_weight_scale);
	ClassDB::bind_method(D_METHOD("remove_point", "id"), &SGAStar2D::remove_point);
	ClassDB::bind_method(D_METHOD("has_point", "id"), &SGAStar2D::has_point);
	ClassDB::bind_method(D_METHOD("get_point_connections", "id"), &SGAStar2D::get_point_connections);
	ClassDB::bind_method(D_METHOD("get_points"), &SGAStar2D::get_points);

	ClassDB::bind_method(D_METHOD("set_point_disabled", "id", "disabled"), &SGAStar2D::set_point_disabled, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_point_disabled", "id"), &SGAStar2D::is_point_disabled);

	ClassDB::bind_method(D_METHOD("connect_points", "id", "to_id", "bidirectional"), &SGAStar2D::connect_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("disconnect_points", "id", "to_id", "bidirectional"), &SGAStar2D::disconnect_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("are_points_connected", "id", "to_id", "bidirectional"), &SGAStar2D::are_points_connected, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("get_point_count"), &SGAStar2D::get_point_count);
	ClassDB::bind_method(D_METHOD("get_point_capacity"), &SGAStar2D::get_point_capacity);
	ClassDB::bind_method(D_METHOD("reserve_space", "num_nodes"), &SGAStar2D::reserve_space);
	ClassDB::bind_method(D_METHOD("clear"), &SGAStar2D::clear);

	ClassDB::bind_method(D_METHOD("get_closest_point", "to_position", "include_disabled"), &SGAStar2D::get_closest_point, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_closest_position_in_segment", "to_position"), &SGAStar2D::get_closest_position_in_segment);

	ClassDB::bind_method(D_METHOD("get_point_path", "from_id", "to_id"), &SGAStar2D::get_point_path);
	ClassDB::bind_method(D_METHOD("get_id_path", "from_id", "to_id"), &SGAStar2D::get_id_path);

	BIND_VMETHOD(MethodInfo(Variant::INT, "_estimate_cost", PropertyInfo(Variant::INT, "from_id"), PropertyInfo(Variant::INT, "to_id")));
	BIND_VMETHOD(MethodInfo(Variant::INT, "_compute_cost", PropertyInfo(Variant::INT, "from_id"), PropertyInfo(Variant::INT, "to_id")));
}

SGAStar2D::SGAStar2D() {
	last_free_id = 0;
	pass = 1;
}

SGAStar2D::~SGAStar2D() {
	clear();
}