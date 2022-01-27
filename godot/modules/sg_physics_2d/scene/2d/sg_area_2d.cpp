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

#include "sg_area_2d.h"

#include <core/engine.h>

#include "sg_static_body_2d.h"
#include "sg_kinematic_body_2d.h"
#include "../../internal/sg_bodies_2d_internal.h"
#include "../../internal/sg_world_2d_internal.h"

void SGArea2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_overlapping_areas", "sort"), &SGArea2D::get_overlapping_areas, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("get_overlapping_bodies", "sort"), &SGArea2D::get_overlapping_bodies, DEFVAL(true));
}

struct SGCollisionObjectComparator {
	bool operator()(const SGCollisionObject2DInternal *p_a, const SGCollisionObject2DInternal *p_b) const {
		SGCollisionObject2D *a = Object::cast_to<SGCollisionObject2D>((Object *)p_a->get_data());
		SGCollisionObject2D *b = Object::cast_to<SGCollisionObject2D>((Object *)p_b->get_data());
		return b->is_greater_than(a);
	}
};

class SGArrayResultHandler : public SGResultHandlerInternal {
private:

	Array result;

public:
	void handle_result(SGCollisionObject2DInternal *p_object) {
		SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>((Object *)p_object->get_data());
		if (object) {
			result.push_back(object);
		}
	}

	_FORCE_INLINE_ Array get_array() {
		return result;
	}

};

class SGSortedArrayResultHandler : public SGResultHandlerInternal {
private:

	List<SGCollisionObject2DInternal *> result;

public:
	void handle_result(SGCollisionObject2DInternal *p_object) {
		result.push_back(p_object);
	}

	_FORCE_INLINE_ Array get_array() {
		if (result.size() > 1) {
			result.sort_custom<SGCollisionObjectComparator>();
		}

		Array ret;
		for (List<SGCollisionObject2DInternal *>::Element *E = result.front(); E; E = E->next()) {
			SGCollisionObject2D *object = Object::cast_to<SGCollisionObject2D>((Object *)E->get()->get_data());
			if (object) {
				ret.push_back(object);
			}
		}

		return ret;
	}

};

Array SGArea2D::get_overlapping_areas(bool sort) const {
	if (sort) {
		SGSortedArrayResultHandler result_handler;
		SGWorld2DInternal::get_singleton()->get_overlapping_areas((SGArea2DInternal *)internal, &result_handler);
		return result_handler.get_array();
	}

	SGArrayResultHandler result_handler;
	SGWorld2DInternal::get_singleton()->get_overlapping_areas((SGArea2DInternal *)internal, &result_handler);
	return result_handler.get_array();
}

Array SGArea2D::get_overlapping_bodies(bool sort) const {
	if (sort) {
		SGSortedArrayResultHandler result_handler;
		SGWorld2DInternal::get_singleton()->get_overlapping_bodies((SGArea2DInternal *)internal, &result_handler);
		return result_handler.get_array();
	}

	SGArrayResultHandler result_handler;
	SGWorld2DInternal::get_singleton()->get_overlapping_bodies((SGArea2DInternal *)internal, &result_handler);
	return result_handler.get_array();
}

SGArea2D::SGArea2D()
	: SGCollisionObject2D(memnew(SGArea2DInternal))
{
}

SGArea2D::~SGArea2D() {
}
