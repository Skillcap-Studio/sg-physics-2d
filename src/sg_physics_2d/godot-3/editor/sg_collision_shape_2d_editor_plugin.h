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

#ifndef SG_COLLISION_SHAPE_2D_EDITOR_PLUGIN_H
#define SG_COLLISION_SHAPE_2D_EDITOR_PLUGIN_H

#include <editor/editor_plugin.h>

class CanvasItemEditor;
class SGCollisionShape2D;

class SGCollisionShape2DEditor : public Control {
	GDCLASS(SGCollisionShape2DEditor, Control);

	enum ShapeType {
		RECTANGLE_SHAPE,
		CIRCLE_SHAPE,
		CAPSULE_SHAPE,
	};

	EditorNode *editor;
	UndoRedo *undo_redo;
	CanvasItemEditor *canvas_item_editor = NULL;
	SGCollisionShape2D *node = NULL;

	Vector<Point2> handles;

	int shape_type;
	int edit_handle;
	bool pressed;
	Variant original;

	Variant get_handle_value(int idx) const;
	void set_handle(int idx, Point2 &p_point);
	void commit_handle(int idx, Variant &p_org);

	void _get_current_shape_type();

protected:
	void _notification(int p_what);
	void _node_removed(Node *p_node);
	static void _bind_methods();

public:
	bool forward_canvas_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);
	void edit(Node *p_node);

	SGCollisionShape2DEditor(EditorNode *p_editor);
};

class SGCollisionShape2DEditorPlugin : public EditorPlugin {
	GDCLASS(SGCollisionShape2DEditorPlugin, EditorPlugin);

	EditorNode *editor;
	SGCollisionShape2DEditor *collision_shape2d_editor;

public:
	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) { return collision_shape2d_editor->forward_canvas_gui_input(p_event); }
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay) { collision_shape2d_editor->forward_canvas_draw_over_viewport(p_overlay); }

	virtual String get_name() const override { return "SGCollisionShape2D"; }
	bool has_main_screen() const override { return false; }
	virtual bool handles(Object *p_obj) const override;
	virtual void edit(Object *p_obj) override;
	virtual void make_visible(bool visible) override;

	SGCollisionShape2DEditorPlugin(EditorNode *p_editor);
	~SGCollisionShape2DEditorPlugin();
};

#endif
