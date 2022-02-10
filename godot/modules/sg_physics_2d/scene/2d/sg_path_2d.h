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

// Code originally from Godot Engine's Path2D (MIT License)

#ifndef SG_PATH_2D_H
#define SG_PATH_2D_H

#include "sg_fixed_node_2d.h"
#include "../resources/sg_curve_2d.h"

class SGPath2D : public SGFixedNode2D {
	GDCLASS(SGPath2D, SGFixedNode2D);

	Ref<SGCurve2D> curve;

	void _curve_changed();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
	virtual bool _edit_is_selected_on_click(const Point2& p_point, double p_tolerance) const;
#endif

	void set_curve(const Ref<SGCurve2D>& p_curve);
	Ref<SGCurve2D> get_curve() const;

	SGPath2D();
};

class SGPathFollow2D : public SGFixedNode2D {
	GDCLASS(SGPathFollow2D, SGFixedNode2D);

public:
private:
	SGPath2D* path;
	fixed offset;
	fixed h_offset;
	fixed v_offset;
	fixed lookahead;
	bool cubic;
	bool loop;
	bool rotate;

	void _update_transform();

protected:
	virtual void _validate_property(PropertyInfo& property) const;

	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_offset(int64_t p_offset);
	int64_t get_offset() const;

	void set_h_offset(int64_t p_h_offset);
	int64_t get_h_offset() const;

	void set_v_offset(int64_t p_v_offset);
	int64_t get_v_offset() const;

	void set_unit_offset(int64_t p_unit_offset);
	int64_t get_unit_offset() const;

	void set_lookahead(int64_t p_lookahead);
	int64_t get_lookahead() const;

	void set_loop(bool p_loop);
	bool has_loop() const;

	void set_rotate(bool p_rotate);
	bool is_rotating() const;

	void set_cubic_interpolation(bool p_enable);
	bool get_cubic_interpolation() const;

	String get_configuration_warning() const;

	SGPathFollow2D();
};

#endif // SG_PATH_2D_H
