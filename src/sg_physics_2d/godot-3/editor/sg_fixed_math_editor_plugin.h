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

#ifndef SG_FIXED_MATH_EDITOR_PLUGIN_H
#define SG_FIXED_MATH_EDITOR_PLUGIN_H

#include <editor/editor_plugin.h>
#include <editor/editor_properties.h>

class FixedEditorSpinSlider : public EditorSpinSlider {
	GDCLASS(FixedEditorSpinSlider, EditorSpinSlider);

	void _evaluate_input_text();

protected:
	static void _bind_methods();

public:
	Control *make_custom_tooltip(const String &p_text) const override;
	void _value_focus_exited();
	FixedEditorSpinSlider();
};

class EditorPropertyFixed : public EditorProperty {
	GDCLASS(EditorPropertyFixed, EditorProperty);
	FixedEditorSpinSlider *spin;
	bool setting;
	void _value_changed(double p_val);

protected:
	static void _bind_methods();

public:
	virtual void update_property();
	void setup(int64_t p_min, int64_t p_max, int64_t p_step, bool p_allow_greater, bool p_allow_lesser);
	EditorPropertyFixed();
};

class SGFixedMathEditorInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(SGFixedMathEditorInspectorPlugin, EditorInspectorPlugin);

public:

	bool can_handle(Object *p_object);
	void parse_begin(Object *p_object);
	bool parse_property(Object *p_object, Variant::Type p_type, const String &p_path, PropertyHint p_hint, const String &p_hint_text, int p_usage);
	void parse_end();
};

class SGFixedMathEditorPlugin : public EditorPlugin {
	GDCLASS(SGFixedMathEditorPlugin, EditorPlugin);

	SGFixedMathEditorInspectorPlugin *fixed_math_editor_inspector_plugin;

protected:

	void _notification(int p_what);

public:

	SGFixedMathEditorPlugin(EditorNode *p_editor);
	~SGFixedMathEditorPlugin();

};

#endif
