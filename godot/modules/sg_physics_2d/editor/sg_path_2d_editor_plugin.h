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

#ifndef SG_PATH_2D_EDITOR_PLUGIN_H
#define SG_PATH_2D_EDITOR_PLUGIN_H

#include <editor/editor_node.h>
#include <editor/editor_plugin.h>
#include <scene/gui/tool_button.h>
#include "../scene/2d/sg_path_2d.h"

class CanvasItemEditor;

class SGPath2DEditor : public HBoxContainer {
	GDCLASS(SGPath2DEditor, HBoxContainer);

	UndoRedo* undo_redo;

	CanvasItemEditor* canvas_item_editor;
	EditorNode* editor;
	Panel* panel;
	SGPath2D* node;

	Separator* sep;

	enum Mode {
		MODE_CREATE,
		MODE_EDIT,
		MODE_EDIT_CURVE,
		MODE_DELETE,
		ACTION_CLOSE
	};

	Mode mode;
	ToolButton* curve_create;
	ToolButton* curve_edit;
	ToolButton* curve_edit_curve;
	ToolButton* curve_del;
	ToolButton* curve_close;
	MenuButton* handle_menu;

	bool mirror_handle_angle;
	bool mirror_handle_length;
	bool on_edge;

	enum HandleOption {
		HANDLE_OPTION_ANGLE,
		HANDLE_OPTION_LENGTH
	};

	enum Action {

		ACTION_NONE,
		ACTION_MOVING_POINT,
		ACTION_MOVING_IN,
		ACTION_MOVING_OUT,
	};

	Action action;
	int action_point;
	Point2 moving_from;
	Point2 moving_screen_from;
	float orig_in_length;
	float orig_out_length;
	Vector2 edge_point;

	void _mode_selected(int p_mode);
	void _handle_option_pressed(int p_option);

	void _node_visibility_changed();
	friend class Path2DEditorPlugin;

protected:
	friend class SGPath2DEditorPlugin;

	HBoxContainer* base_hb;

	void _notification(int p_what);
	void _node_removed(Node* p_node);
	static void _bind_methods();

public:
	bool forward_gui_input(const Ref<InputEvent>& p_event);
	void forward_canvas_draw_over_viewport(Control* p_overlay);
	void edit(Node* p_path2d);
	SGPath2DEditor(EditorNode* p_editor);
};

class SGPath2DEditorPlugin : public EditorPlugin {
	GDCLASS(SGPath2DEditorPlugin, EditorPlugin);

	SGPath2DEditor* path2d_editor;
	EditorNode* editor;

public:
	virtual bool forward_canvas_gui_input(const Ref<InputEvent>& p_event) { return path2d_editor->forward_gui_input(p_event); }
	virtual void forward_canvas_draw_over_viewport(Control* p_overlay) { path2d_editor->forward_canvas_draw_over_viewport(p_overlay); }

	virtual String get_name() const { return "Path2D"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object* p_object);
	virtual bool handles(Object* p_object) const;
	virtual void make_visible(bool p_visible);

	SGPath2DEditorPlugin(EditorNode* p_node);
	~SGPath2DEditorPlugin();
};

#endif // SG_PATH_2D_EDITOR_PLUGIN_H
