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
		spin[i] = memnew(EditorSpinSlider);
		spin[i]->set_flat(true);
		spin[i]->set_label(desc[i]);
		bc->add_child(spin[i]);
		add_focusable(spin[i]);
		spin[i]->connect("value_changed", this, "_value_changed", varray(desc[i]));
		if (horizontal) {
			spin[i]->set_h_size_flags(SIZE_EXPAND_FILL);
		}

		spin[i]->set_min(-65535);
		spin[i]->set_max(65535);
		spin[i]->set_step(1);
		spin[i]->set_hide_slider(true);
		spin[i]->set_allow_greater(true);
		spin[i]->set_allow_lesser(true);
		spin[i]->set_use_rounded_values(true);
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
