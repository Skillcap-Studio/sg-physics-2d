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

#include "sg_curve_2d.h"

#include "core/core_string_names.h"

template <class T>
static _FORCE_INLINE_ T _bezier_interp(fixed t, T start, T control_1, T control_2, T end) {
	/* Formula from Wikipedia article on Bezier curves. */
	fixed omt = (fixed::ONE - t);
	fixed omt2 = omt * omt;
	fixed omt3 = omt2 * omt;
	fixed t2 = t * t;
	fixed t3 = t2 * t;

	return start * omt3 + control_1 * omt2 * t * fixed(196608) + control_2 * omt * t2 * fixed(196608) + end * t3;
}

int SGCurve2D::get_point_count() const {
	return points.size();
}

void SGCurve2D::add_point(Ref<SGFixedVector2> p_pos, Ref<SGFixedVector2> p_in, Ref<SGFixedVector2> p_out, int p_atpos) {
	Point n;
	n.pos = p_pos->get_internal();

	if (p_in.is_valid()) {
		n.in = p_in->get_internal();
	}
	else {
		n.in = SGFixedVector2Internal();
	}

	if (p_out.is_valid()) {
		n.out = p_out->get_internal();
	}
	else {
		n.out = SGFixedVector2Internal();
	}

	if (p_atpos >= 0 && p_atpos < points.size()) {
		points.insert(p_atpos, n);
	}
	else {
		points.push_back(n);
	}

	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}

void SGCurve2D::set_point_position(int p_index, Ref<SGFixedVector2> p_pos) {
	ERR_FAIL_INDEX(p_index, points.size());

	points.write[p_index].pos = p_pos->get_internal();
	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}
SGFixedVector2Internal SGCurve2D::get_point_position_internal(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), SGFixedVector2Internal());
	return points[p_index].pos;
}
Ref<SGFixedVector2> SGCurve2D::get_point_position(int p_index) const {
	return SGFixedVector2::from_internal(get_point_position_internal(p_index));
}

void SGCurve2D::set_point_in(int p_index, Ref<SGFixedVector2> p_in) {
	ERR_FAIL_INDEX(p_index, points.size());

	points.write[p_index].in = p_in->get_internal();
	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}
Ref<SGFixedVector2> SGCurve2D::get_point_in(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), nullptr);
	return SGFixedVector2::from_internal(points[p_index].in);
}

void SGCurve2D::set_point_out(int p_index, Ref<SGFixedVector2> p_out) {
	ERR_FAIL_INDEX(p_index, points.size());

	points.write[p_index].out = p_out->get_internal();
	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}

Ref<SGFixedVector2> SGCurve2D::get_point_out(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), nullptr);
	return SGFixedVector2::from_internal(points[p_index].out);
}

void SGCurve2D::remove_point(int p_index) {
	ERR_FAIL_INDEX(p_index, points.size());
	points.remove(p_index);
	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}

void SGCurve2D::clear_points() {
	if (!points.empty()) {
		points.clear();
		baked_cache_dirty = true;
		emit_signal(CoreStringNames::get_singleton()->changed);
	}
}

SGFixedVector2Internal SGCurve2D::interpolate_internal(int p_index, fixed p_offset) const {
	int pc = points.size();
	ERR_FAIL_COND_V(pc == 0, SGFixedVector2Internal());

	if (p_index >= pc - 1) {
		return points[pc - 1].pos;
	}
	else if (p_index < 0) {
		return points[0].pos;
	}

	SGFixedVector2Internal p0 = points[p_index].pos;
	SGFixedVector2Internal p1 = p0 + points[p_index].out;
	SGFixedVector2Internal p3 = points[p_index + 1].pos;
	SGFixedVector2Internal p2 = p3 + points[p_index + 1].in;

	return _bezier_interp(p_offset, p0, p1, p2, p3);
}
Ref<SGFixedVector2> SGCurve2D::interpolate(int p_index, int64_t p_offset) const {
	return SGFixedVector2::from_internal(interpolate_internal(p_index, fixed(p_offset)));
}

SGFixedVector2Internal SGCurve2D::interpolatef_internal(fixed p_findex) const {
	if (p_findex < fixed(0)) {
		p_findex = fixed(0);
	}
	else if (p_findex >= fixed::from_int(points.size())) {
		p_findex = fixed::from_int(points.size());
	}

	return interpolate_internal(p_findex.to_int(), p_findex % fixed::ONE);
}
Ref<SGFixedVector2> SGCurve2D::interpolatef(int64_t p_findex) const {
	return SGFixedVector2::from_internal(interpolatef_internal(fixed(p_findex)));
}

void SGCurve2D::_bake_segment2d(Map<fixed, SGFixedVector2Internal>& r_bake, fixed p_begin, fixed p_end, const SGFixedVector2Internal& p_a, const SGFixedVector2Internal& p_out, const SGFixedVector2Internal& p_b, const SGFixedVector2Internal& p_in, int p_depth, int p_max_depth, fixed p_tol) const {
	fixed mp = p_begin + ((p_end - p_begin) >> 1);
	SGFixedVector2Internal beg = _bezier_interp(p_begin, p_a, p_a + p_out, p_b + p_in, p_b);
	SGFixedVector2Internal mid = _bezier_interp(mp, p_a, p_a + p_out, p_b + p_in, p_b);
	SGFixedVector2Internal end = _bezier_interp(p_end, p_a, p_a + p_out, p_b + p_in, p_b);

	SGFixedVector2Internal na = (mid - beg).normalized();
	SGFixedVector2Internal nb = (end - mid).normalized();
	fixed dp = na.dot(nb);

	if (dp < p_tol.cos()) {
		r_bake[mp] = mid;
	}

	if (p_depth < p_max_depth) {
		_bake_segment2d(r_bake, p_begin, mp, p_a, p_out, p_b, p_in, p_depth + 1, p_max_depth, p_tol);
		_bake_segment2d(r_bake, mp, p_end, p_a, p_out, p_b, p_in, p_depth + 1, p_max_depth, p_tol);
	}
}

void SGCurve2D::_bake() const {
	if (!baked_cache_dirty) {
		return;
	}

	baked_max_ofs = fixed(0);
	baked_cache_dirty = false;

	if (points.size() == 0) {
		baked_point_cache.resize(0);
		return;
	}

	if (points.size() == 1) {
		baked_point_cache.resize(1);
		baked_point_cache.set(0, points[0].pos);
		return;
	}

	SGFixedVector2Internal pos = points[0].pos;
	List<SGFixedVector2Internal> pointlist;

	pointlist.push_back(pos); //start always from origin

	for (int i = 0; i < points.size() - 1; i++) {
		fixed step(6553); //0.1 // at least 10 substeps ought to be enough?
		fixed p(0);

		while (p < fixed::ONE) {
			fixed np = p + step;
			if (np > fixed::ONE) {
				np = fixed::ONE;
			}

			SGFixedVector2Internal npp = _bezier_interp(np, points[i].pos, points[i].pos + points[i].out, points[i + 1].pos + points[i + 1].in, points[i + 1].pos);
			fixed d = pos.distance_to(npp);

			if (d > bake_interval) {
				// OK! between P and NP there _has_ to be Something, let's go searching!

				int iterations = 10; //lots of detail!

				fixed low = p;
				fixed hi = np;
				fixed mid = low + ((hi - low) >> 1);

				for (int j = 0; j < iterations; j++) {
					npp = _bezier_interp(mid, points[i].pos, points[i].pos + points[i].out, points[i + 1].pos + points[i + 1].in, points[i + 1].pos);
					d = pos.distance_to(npp);

					if (bake_interval < d) {
						hi = mid;
					}
					else {
						low = mid;
					}
					mid = low + ((hi - low) >> 1);
				}

				pos = npp;
				p = mid;
				pointlist.push_back(pos);
			}
			else {
				p = np;
			}
		}
	}

	SGFixedVector2Internal lastpos = points[points.size() - 1].pos;

	fixed rem = pos.distance_to(lastpos);
	baked_max_ofs = fixed::from_int(pointlist.size() - 1) * bake_interval + rem;
	pointlist.push_back(lastpos);

	baked_point_cache.resize(pointlist.size());
	int idx = 0;

	for (List<SGFixedVector2Internal>::Element* E = pointlist.front(); E; E = E->next()) {
		baked_point_cache.write[idx] = E->get();
		idx++;
	}
}

int64_t SGCurve2D::get_baked_length() const {
	if (baked_cache_dirty) {
		_bake();
	}

	return baked_max_ofs.value;
}
SGFixedVector2Internal SGCurve2D::interpolate_baked_internal(fixed p_offset, bool p_cubic) const {
	if (baked_cache_dirty) {
		_bake();
	}

	//validate//
	int pc = baked_point_cache.size();
	ERR_FAIL_COND_V_MSG(pc == 0, SGFixedVector2Internal(), "No points in Curve2D.");

	if (pc == 1) {
		return baked_point_cache.get(0);
	}

	int bpc = baked_point_cache.size();

	if (p_offset < fixed(0)) {
		return baked_point_cache[0];
	}
	if (p_offset >= baked_max_ofs) {
		return baked_point_cache[bpc - 1];
	}

	int idx = (p_offset / bake_interval).to_int();
	fixed frac = p_offset % bake_interval;

	if (idx >= bpc - 1) {
		return baked_point_cache[bpc - 1];
	}
	else if (idx == bpc - 2) {
		if (frac > fixed(0)) {
			frac /= baked_max_ofs % bake_interval;
		}
	}
	else {
		frac /= bake_interval;
	}

	if (p_cubic) {
		SGFixedVector2Internal pre = idx > 0 ? baked_point_cache[idx - 1] : baked_point_cache[idx];
		SGFixedVector2Internal post = (idx < (bpc - 2)) ? baked_point_cache[idx + 2] : baked_point_cache[idx + 1];
		return baked_point_cache[idx].cubic_interpolate(baked_point_cache[idx + 1], pre, post, frac);
	}
	else {
		return SGFixedVector2Internal::linear_interpolate(baked_point_cache[idx], baked_point_cache[idx + 1], frac);
	}
}
Ref<SGFixedVector2> SGCurve2D::interpolate_baked(int64_t p_offset, bool p_cubic) const {
	return SGFixedVector2::from_internal(interpolate_baked_internal(fixed(p_offset), p_cubic));
}

Vector<SGFixedVector2Internal> SGCurve2D::get_baked_points_internal() const {
	if (baked_cache_dirty) {
		_bake();
	}

	return baked_point_cache;
}
Array SGCurve2D::get_baked_points() const {
	Vector<SGFixedVector2Internal> baked_points_internal = get_baked_points_internal();
	Array a;
	a.resize(baked_points_internal.size());
	for (int i = 0; i < baked_points_internal.size(); i++) {
		a[i] = (SGFixedVector2::from_internal(baked_points_internal[i]));
	}
	return a;
}

void SGCurve2D::set_bake_interval_internal(fixed p_tolerance) {
	bake_interval = p_tolerance;
	baked_cache_dirty = true;
	emit_signal(CoreStringNames::get_singleton()->changed);
}
void SGCurve2D::set_bake_interval(int64_t p_tolerance) {
	set_bake_interval_internal(fixed(p_tolerance));
}

int64_t SGCurve2D::get_bake_interval() const {
	return bake_interval.value;
}

SGFixedVector2Internal SGCurve2D::get_closest_point_internal(const SGFixedVector2Internal& p_to_point) const {
	// Brute force method

	if (baked_cache_dirty) {
		_bake();
	}

	//validate//
	int pc = baked_point_cache.size();
	ERR_FAIL_COND_V_MSG(pc == 0, SGFixedVector2Internal(), "No points in Curve2D.");

	if (pc == 1) {
		return baked_point_cache.get(0);
	}

	SGFixedVector2Internal nearest;
	fixed nearest_dist = fixed::NEG_ONE;

	for (int i = 0; i < pc - 1; i++) {
		SGFixedVector2Internal origin = baked_point_cache[i];
		SGFixedVector2Internal direction = (baked_point_cache[i + 1] - origin) / bake_interval;

		fixed d = CLAMP((p_to_point - origin).dot(direction), fixed(0), bake_interval);
		SGFixedVector2Internal proj = origin + direction * d;

		fixed dist = proj.distance_squared_to(p_to_point);

		if (nearest_dist < fixed(0) || dist < nearest_dist) {
			nearest = proj;
			nearest_dist = dist;
		}
	}

	return nearest;
}
Ref<SGFixedVector2> SGCurve2D::get_closest_point(Ref<SGFixedVector2> p_to_point) const {
	return SGFixedVector2::from_internal(get_closest_point_internal(p_to_point->get_internal()));
}

fixed SGCurve2D::get_closest_offset_internal(const SGFixedVector2Internal& p_to_point) const {
	// Brute force method

	if (baked_cache_dirty) {
		_bake();
	}

	//validate//
	int pc = baked_point_cache.size();
	ERR_FAIL_COND_V_MSG(pc == 0, fixed(0), "No points in Curve2D.");

	if (pc == 1) {
		return fixed(0);
	}

	fixed nearest(0);
	fixed nearest_dist = fixed::NEG_ONE;
	fixed offset(0);

	for (int i = 0; i < pc - 1; i++) {
		SGFixedVector2Internal origin = baked_point_cache[i];
		SGFixedVector2Internal direction = (baked_point_cache[i + 1] - origin) / bake_interval;

		fixed d = CLAMP((p_to_point - origin).dot(direction), fixed(0), bake_interval);
		SGFixedVector2Internal proj = origin + direction * d;

		fixed dist = proj.distance_squared_to(p_to_point);

		if (nearest_dist < fixed(0) || dist < nearest_dist) {
			nearest = offset + d;
			nearest_dist = dist;
		}

		offset += bake_interval;
	}

	return nearest;
}
int64_t SGCurve2D::get_closest_offset(Ref<SGFixedVector2> p_to_point) const {
	return get_closest_offset_internal(p_to_point->get_internal()).value;
}

Dictionary SGCurve2D::_get_data() const {
	Dictionary dc;

	Array d;
	d.resize(points.size() * 3);

	for (int i = 0; i < points.size(); i++) {
		{
			Ref<SGFixedVector2> p = SGFixedVector2::from_internal(points[i].in);
			if (p.is_valid()) {
				Array pair;
				pair.resize(2);
				pair[0] = p->get_x();
				pair[1] = p->get_y();
				d[i * 3 + 0] = pair;
			}
		}
		{
			Ref<SGFixedVector2> p = SGFixedVector2::from_internal(points[i].out);
			if (p.is_valid()) {
				Array pair;
				pair.resize(2);
				pair[0] = p->get_x();
				pair[1] = p->get_y();
				d[i * 3 + 1] = pair;
			}
		}
		{
			Ref<SGFixedVector2> p = SGFixedVector2::from_internal(points[i].pos);
			if (p.is_valid()) {
				Array pair;
				pair.resize(2);
				pair[0] = p->get_x();
				pair[1] = p->get_y();
				d[i * 3 + 2] = pair;
			}
		}
	}

	dc["points"] = d;

	return dc;
}
void SGCurve2D::_set_data(const Dictionary& p_data) {
	ERR_FAIL_COND(!p_data.has("points"));

	Array rp = p_data["points"];
	int pc = rp.size();
	ERR_FAIL_COND(pc % 3 != 0);
	points.resize(pc / 3);

	for (int i = 0; i < points.size(); i++) {
		Array pair = rp[i * 3 + 0];
		if (pair.size() == 2) {
			int64_t x = pair[0];
			int64_t y = pair[1];
			SGFixedVector2Internal point = SGFixedVector2Internal(fixed(x), fixed(y));
			points.write[i].in = point;
		}

		pair = rp[i * 3 + 1];
		if (pair.size() == 2) {
			int64_t x = pair[0];
			int64_t y = pair[1];
			SGFixedVector2Internal point = SGFixedVector2Internal(fixed(x), fixed(y));
			points.write[i].out = point;
		}

		pair = rp[i * 3 + 2];
		if (pair.size() == 2) {
			int64_t x = pair[0];
			int64_t y = pair[1];
			SGFixedVector2Internal point = SGFixedVector2Internal(fixed(x), fixed(y));
			points.write[i].pos = point;
		}
	}

	baked_cache_dirty = true;
}

Vector<SGFixedVector2Internal> SGCurve2D::tessellate_internal(int p_max_stages, fixed p_tolerance) const {
	Vector<SGFixedVector2Internal> tess;

	if (points.size() == 0) {
		return tess;
	}
	Vector<Map<fixed, SGFixedVector2Internal>> midpoints;

	midpoints.resize(points.size() - 1);

	int pc = 1;
	for (int i = 0; i < points.size() - 1; i++) {
		_bake_segment2d(midpoints.write[i], fixed(0), fixed(1), points[i].pos, points[i].out, points[i + 1].pos, points[i + 1].in, 0, p_max_stages, p_tolerance);
		pc++;
		pc += midpoints[i].size();
	}

	tess.resize(pc);
	tess.write[0] = points[0].pos;
	int pidx = 0;

	for (int i = 0; i < points.size() - 1; i++) {
		for (Map<fixed, SGFixedVector2Internal>::Element* E = midpoints[i].front(); E; E = E->next()) {
			pidx++;
			tess.write[pidx] = E->get();
		}

		pidx++;
		tess.write[pidx] = points[i + 1].pos;
	}

	return tess;
}
Array SGCurve2D::tessellate(int p_max_stages, int64_t p_tolerance) const {
	Vector<SGFixedVector2Internal> tess = tessellate_internal(p_max_stages, fixed(p_tolerance));
	Array a;
	a.resize(tess.size());
	for (int i = 0; i < tess.size(); i++) {
		a[i] = (SGFixedVector2::from_internal(tess[i]));
	}
	return a;
}

void SGCurve2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_point_count"), &SGCurve2D::get_point_count);
	ClassDB::bind_method(D_METHOD("add_point", "position", "in", "out", "at_position"), &SGCurve2D::add_point, DEFVAL(Variant()), DEFVAL(Variant()), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("set_point_position", "idx", "position"), &SGCurve2D::set_point_position);
	ClassDB::bind_method(D_METHOD("get_point_position", "idx"), &SGCurve2D::get_point_position);
	ClassDB::bind_method(D_METHOD("set_point_in", "idx", "position"), &SGCurve2D::set_point_in);
	ClassDB::bind_method(D_METHOD("get_point_in", "idx"), &SGCurve2D::get_point_in);
	ClassDB::bind_method(D_METHOD("set_point_out", "idx", "position"), &SGCurve2D::set_point_out);
	ClassDB::bind_method(D_METHOD("get_point_out", "idx"), &SGCurve2D::get_point_out);
	ClassDB::bind_method(D_METHOD("remove_point", "idx"), &SGCurve2D::remove_point);
	ClassDB::bind_method(D_METHOD("clear_points"), &SGCurve2D::clear_points);
	ClassDB::bind_method(D_METHOD("interpolate", "idx", "t"), &SGCurve2D::interpolate);
	ClassDB::bind_method(D_METHOD("interpolatef", "fofs"), &SGCurve2D::interpolatef);
	ClassDB::bind_method(D_METHOD("set_bake_interval", "distance"), &SGCurve2D::set_bake_interval);
	ClassDB::bind_method(D_METHOD("get_bake_interval"), &SGCurve2D::get_bake_interval);

	ClassDB::bind_method(D_METHOD("get_baked_length"), &SGCurve2D::get_baked_length);
	ClassDB::bind_method(D_METHOD("interpolate_baked", "offset", "cubic"), &SGCurve2D::interpolate_baked, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_baked_points"), &SGCurve2D::get_baked_points);
	ClassDB::bind_method(D_METHOD("get_closest_point", "to_point"), &SGCurve2D::get_closest_point);
	ClassDB::bind_method(D_METHOD("get_closest_offset", "to_point"), &SGCurve2D::get_closest_offset);
	ClassDB::bind_method(D_METHOD("tessellate", "max_stages", "tolerance_degrees"), &SGCurve2D::tessellate, DEFVAL(5), DEFVAL(4));

	ClassDB::bind_method(D_METHOD("_get_data"), &SGCurve2D::_get_data);
	ClassDB::bind_method(D_METHOD("_set_data"), &SGCurve2D::_set_data);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "bake_interval", PROPERTY_HINT_RANGE, "0.01,512,0.01"), "set_bake_interval", "get_bake_interval");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_data", "_get_data");
}

SGCurve2D::SGCurve2D() {
	baked_cache_dirty = false;
	baked_max_ofs = fixed(0);
	bake_interval = fixed(327680);
}
