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

#include "sg_fixed_math_editor_plugin.h"

#include "../scene/resources/sg_shapes_2d.h"
#include <core/math/expression.h>

class TooltipPanel : public PanelContainer {
	GDCLASS(TooltipPanel, PanelContainer);

public:
	TooltipPanel(){};
};

class TooltipLabel : public Label {
	GDCLASS(TooltipLabel, Label);

public:
	TooltipLabel(){};
};

Control *FixedEditorSpinSlider::make_custom_tooltip(const String &p_text) const {
	String text = vformat("%s\nFloat: %s", get_value(), get_value() / 65536.);
	TooltipPanel *tooltip_popup = memnew(TooltipPanel);
	TooltipLabel *tooltip_label = memnew(TooltipLabel);
	tooltip_popup->add_child(tooltip_label);

	Ref<StyleBox> ttp = tooltip_label->get_stylebox("panel", "TooltipPanel");

	tooltip_label->set_anchor_and_margin(MARGIN_LEFT, Control::ANCHOR_BEGIN, ttp->get_margin(MARGIN_LEFT));
	tooltip_label->set_anchor_and_margin(MARGIN_TOP, Control::ANCHOR_BEGIN, ttp->get_margin(MARGIN_TOP));
	tooltip_label->set_anchor_and_margin(MARGIN_RIGHT, Control::ANCHOR_END, -ttp->get_margin(MARGIN_RIGHT));
	tooltip_label->set_anchor_and_margin(MARGIN_BOTTOM, Control::ANCHOR_END, -ttp->get_margin(MARGIN_BOTTOM));
	tooltip_label->set_text(text);
	return tooltip_popup;
}

FixedEditorSpinSlider::FixedEditorSpinSlider() {
	set_step(0);
	set_allow_greater(true);
	set_allow_lesser(true);
	get_line_edit()->disconnect("focus_exited", this, "_value_focus_exited");
	get_line_edit()->connect("focus_exited", this, "_value_focus_exited_fixed");
}

void FixedEditorSpinSlider::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_value_focus_exited_fixed"), &FixedEditorSpinSlider::_value_focus_exited);
}

void FixedEditorSpinSlider::_value_focus_exited() {
	LineEdit *value_input = get_line_edit();
	// discontinue because the focus_exit was caused by right-click context menu
	if (value_input->get_menu()->is_visible()) {
		return;
	}

	_evaluate_input_text();
}

void FixedEditorSpinSlider::_evaluate_input_text() {
	LineEdit *value_input = get_line_edit();
	// Replace comma with dot to support it as decimal separator (GH-6028).
	// This prevents using functions like `pow()`, but using functions
	// in EditorSpinSlider is a barely known (and barely used) feature.
	// Instead, we'd rather support German/French keyboard layouts out of the box.
	const String text = value_input->get_text().replace(",", ".");

	Ref<Expression> expr;
	expr.instance();
	Error err = expr->parse(text);
	if (err != OK) {
		return;
	}

	Variant v = expr->execute(Array(), nullptr, false);
	if (v.get_type() == Variant::NIL) {
		return;
	} else if (v.get_type() == Variant::REAL) {
		v = (int)((real_t)v * 65536);
	}
	set_value(v);
}

void EditorPropertyFixed::_value_changed(double val) {
	if (setting) {
		return;
	}
	emit_changed(get_edited_property(), (int)val);
}

void EditorPropertyFixed::update_property() {
	int64_t val = get_edited_object()->get(get_edited_property());
	setting = true;
	spin->set_value(val);
	setting = false;
#ifdef DEBUG_ENABLED
	// If spin (currently EditorSplinSlider : Range) is changed so that it can use int64_t, then the below warning wouldn't be a problem.
	if (val != (int64_t)(double)(val)) {
		WARN_PRINT("Cannot reliably represent '" + itos(val) + "' in the inspector, value is too large.");
	}
#endif
}

void EditorPropertyFixed::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_value_changed"), &EditorPropertyFixed::_value_changed);
}

void EditorPropertyFixed::setup(int64_t p_min, int64_t p_max, int64_t p_step, bool p_allow_greater, bool p_allow_lesser) {
	spin->set_min(p_min);
	spin->set_max(p_max);
	spin->set_step(p_step);
	spin->set_allow_greater(p_allow_greater);
	spin->set_allow_lesser(p_allow_lesser);
}

EditorPropertyFixed::EditorPropertyFixed() {
	spin = memnew(FixedEditorSpinSlider);
	spin->set_flat(true);
	add_child(spin);
	add_focusable(spin);
	spin->connect("value_changed", this, "_value_changed");
	setting = false;
}

void EditorPropertySGFixedVector2::_value_changed(double val, const String &p_name) {
	if (setting)
		return;

	Ref<SGFixedVector2> v2(memnew(SGFixedVector2()));
	v2->set_x((int)spin[0]->get_value());
	v2->set_y((int)spin[1]->get_value());
	emit_changed(get_edited_property(), v2, p_name);
}

void EditorPropertySGFixedVector2::update_property() {
	Ref<SGFixedVector2> val = get_edited_object()->get(get_edited_property());
	setting = true;
	spin[0]->set_value(val->get_x());
	spin[1]->set_value(val->get_y());
	setting = false;
}

void EditorPropertySGFixedVector2::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE || p_what == NOTIFICATION_THEME_CHANGED) {
		Color base = get_color("accent_color", "Editor");
		for (int i = 0; i < 2; i++) {

			Color c = base;
			c.set_hsv(float(i) / 3.0 + 0.05, c.get_s() * 0.75, c.get_v());
			spin[i]->set_custom_label_color(true, c);
		}
	}
}

void EditorPropertySGFixedVector2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_value_changed"), &EditorPropertySGFixedVector2::_value_changed);
}

EditorPropertySGFixedVector2::EditorPropertySGFixedVector2() {
	bool horizontal = EDITOR_GET("interface/inspector/horizontal_vector2_editing");

	BoxContainer *bc;

	if (horizontal) {
		bc = memnew(HBoxContainer);
		add_child(bc);
		set_bottom_editor(bc);
	} else {
		bc = memnew(VBoxContainer);
		add_child(bc);
	}

	static const char *desc[2] = { "x", "y" };
	for (int i = 0; i < 2; i++) {
		spin[i] = memnew(FixedEditorSpinSlider);
		spin[i]->set_flat(true);
		spin[i]->set_label(desc[i]);
		bc->add_child(spin[i]);
		add_focusable(spin[i]);
		spin[i]->connect("value_changed", this, "_value_changed", varray(desc[i]));
		if (horizontal) {
			spin[i]->set_h_size_flags(SIZE_EXPAND_FILL);
		}
	}

	if (!horizontal) {
		set_label_reference(spin[0]); //show text and buttons around this
	}
	setting = false;
}


bool SGFixedMathEditorInspectorPlugin::can_handle(Object *p_object) {
	return true;
}

void SGFixedMathEditorInspectorPlugin::parse_begin(Object *p_object) {
}

bool SGFixedMathEditorInspectorPlugin::parse_property(Object *p_object, Variant::Type p_type, const String &p_path, PropertyHint p_hint, const String &p_hint_text, int p_usage) {
	if (p_hint == PROPERTY_HINT_TYPE_STRING && p_hint_text == "SGFixedVector2") {
		EditorPropertySGFixedVector2 *editor = memnew(EditorPropertySGFixedVector2);
		add_property_editor(p_path, editor);
		return true;
	}
	if (p_type == Variant::INT && p_hint == PROPERTY_HINT_NONE) {
		EditorPropertyFixed *editor = memnew(EditorPropertyFixed);
		add_property_editor(p_path, editor);
		return true;
	}
	return false;
}

void SGFixedMathEditorInspectorPlugin::parse_end() {
}

SGFixedMathEditorPlugin::SGFixedMathEditorPlugin(EditorNode *p_editor) {
	fixed_math_editor_inspector_plugin = memnew(SGFixedMathEditorInspectorPlugin);
}

SGFixedMathEditorPlugin::~SGFixedMathEditorPlugin() {
}

void SGFixedMathEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			add_inspector_plugin(fixed_math_editor_inspector_plugin);
			break;
		
		case NOTIFICATION_EXIT_TREE:
			remove_inspector_plugin(fixed_math_editor_inspector_plugin);
			break;
	}
}
