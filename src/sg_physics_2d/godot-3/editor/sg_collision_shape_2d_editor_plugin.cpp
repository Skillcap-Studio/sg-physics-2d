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

#include "sg_collision_shape_2d_editor_plugin.h"

#include <editor/editor_node.h>
#include <editor/plugins/canvas_item_editor_plugin.h>
#include "../scene/2d/sg_collision_shape_2d.h"
#include "../scene/resources/sg_shapes_2d.h"

Variant SGCollisionShape2DEditor::get_handle_value(int idx) const {
	switch (shape_type) {
		case RECTANGLE_SHAPE: {
			Ref<SGRectangleShape2D> rectangle = node->get_shape();

			if (idx < 3) {
				return rectangle->get_extents()->abs();
			}
		} break;

		case CIRCLE_SHAPE: {
			Ref<SGCircleShape2D> circle = node->get_shape();

			if (idx == 0) {
				return circle->get_radius();
			}
		} break;

		case CAPSULE_SHAPE: {
			Ref<SGCapsuleShape2D> capsule = node->get_shape();

			if (idx == 0) {
				return capsule->get_radius();
			}
			else if (idx == 1) {
				return capsule->get_height();
			}
		} break;
	}

	return Variant();
}

void SGCollisionShape2DEditor::set_handle(int idx, Point2 &p_point) {
	switch (shape_type) {
		case RECTANGLE_SHAPE: {
			Ref<SGRectangleShape2D> rectangle = node->get_shape();

			Ref<SGFixedVector2> extents = rectangle->get_extents();
			switch (idx) {
				case 0:
					extents->set_x(fixed::from_float(p_point.x).value);
					break;
				case 1:
					extents->set_y(fixed::from_float(p_point.y).value);
					break;
				case 2:
					extents->from_float(p_point);
					break;
			}
			rectangle->set_extents(extents->abs());

			canvas_item_editor->update_viewport();
		} break;

		case CIRCLE_SHAPE: {
			Ref<SGCircleShape2D> circle = node->get_shape();
			circle->set_radius(fixed::from_float(p_point.length()).value);

			canvas_item_editor->update_viewport();
		} break;

		case CAPSULE_SHAPE: {
			Ref<SGCapsuleShape2D> capsule = node->get_shape();
			switch (idx) {
				case 0:
					capsule->set_radius(fixed::from_float(p_point.length()).value);
					break;
				case 1:
					capsule->set_height(fixed::from_float(p_point.length()).value);
					break;
			}

			canvas_item_editor->update_viewport();
		} break;
	}
}

void SGCollisionShape2DEditor::commit_handle(int idx, Variant &p_org) {
	undo_redo->create_action(TTR("Set Handle"));

	switch (shape_type) {
		case RECTANGLE_SHAPE: {
			Ref<SGRectangleShape2D> rectangle = node->get_shape();

			undo_redo->add_do_method(rectangle.ptr(), "set_extents", rectangle->get_extents());
			undo_redo->add_do_method(canvas_item_editor, "update_viewport");
			undo_redo->add_undo_method(rectangle.ptr(), "set_extents", p_org);
			undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
		} break;

		case CIRCLE_SHAPE: {
			Ref<SGCircleShape2D> circle = node->get_shape();

			undo_redo->add_do_method(circle.ptr(), "set_radius", circle->get_radius());
			undo_redo->add_do_method(canvas_item_editor, "update_viewport");
			undo_redo->add_undo_method(circle.ptr(), "set_radius", p_org);
			undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
		} break;

		case CAPSULE_SHAPE: {
			Ref<SGCapsuleShape2D> capsule = node->get_shape();

			undo_redo->add_do_method(capsule.ptr(), "set_radius", capsule->get_radius());
			undo_redo->add_do_method(canvas_item_editor, "update_viewport");
			undo_redo->add_undo_method(capsule.ptr(), "set_radius", p_org);
			undo_redo->add_undo_method(canvas_item_editor, "update_viewport");
		} break;
	}

	undo_redo->commit_action();
}

void SGCollisionShape2DEditor::_get_current_shape_type() {
	if (!node) {
		return;
	}

	Ref<SGShape2D> shape = node->get_shape();

	if (!shape.is_valid()) {
		return;
	}

	if (Object::cast_to<SGRectangleShape2D>(*shape)) {
		shape_type = RECTANGLE_SHAPE;
	}
	else if (Object::cast_to<SGCircleShape2D>(*shape)) {
		shape_type = CIRCLE_SHAPE;
	}
	else if (Object::cast_to<SGCapsuleShape2D>(*shape)) {
		shape_type = CAPSULE_SHAPE;
	}
	else {
		shape_type = -1;
	}

	canvas_item_editor->update_viewport();
}

void SGCollisionShape2DEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			get_tree()->connect("node_removed", this, "_node_removed");
			break;

		case NOTIFICATION_EXIT_TREE:
			get_tree()->disconnect("node_removed", this, "_node_removed");
			break;
	}
}

void SGCollisionShape2DEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = nullptr;
	}
}

void SGCollisionShape2DEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_get_current_shape_type"), &SGCollisionShape2DEditor::_get_current_shape_type);
	ClassDB::bind_method(D_METHOD("_node_removed"), &SGCollisionShape2DEditor::_node_removed);
}

bool SGCollisionShape2DEditor::forward_canvas_gui_input(const Ref<InputEvent> &p_event) {
	if (!node) {
		return false;
	}

	if (!node->get_shape().is_valid()) {
		return false;
	}

	if (shape_type == -1) {
		return false;
	}

	Ref<InputEventMouseButton> mb = p_event;
	Transform2D xform = canvas_item_editor->get_canvas_transform() * node->get_global_transform();

	if (mb.is_valid()) {
		Vector2 gpoint = mb->get_position();

		if (mb->get_button_index() == BUTTON_LEFT) {
			if (mb->is_pressed()) {
				for (int i = 0; i < handles.size(); i++) {
					if (xform.xform(handles[i]).distance_to(gpoint) < 8) {
						edit_handle = i;
						break;
					}
				}

				if (edit_handle == -1) {
					pressed = false;
					return false;
				}

				original = get_handle_value(edit_handle);
				pressed = true;

				return true;
			}
			else {
				if (pressed) {
					commit_handle(edit_handle, original);

					edit_handle = -1;
					pressed = false;

					return true;
				}
			}
		}

		return false;
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		if (edit_handle == -1 || !pressed) {
			return false;
		}

		Vector2 cpoint = canvas_item_editor->snap_point(canvas_item_editor->get_canvas_transform().affine_inverse().xform(mm->get_position()));
		cpoint = node->get_global_transform().affine_inverse().xform(cpoint);

		set_handle(edit_handle, cpoint);

		return true;
	}

	return false;
}

void SGCollisionShape2DEditor::forward_canvas_draw_over_viewport(Control *p_overlay) {
	if (!node) {
		return;
	}

	if (!node->get_shape().is_valid()) {
		return;
	}

	_get_current_shape_type();

	if (shape_type == -1) {
		return;
	}

	Transform2D gt = canvas_item_editor->get_canvas_transform() * node->get_global_transform();

	Ref<Texture> h = get_icon("EditorHandle", "EditorIcons");
	Vector2 size = h->get_size() * 0.5;

	handles.clear();

	switch (shape_type) {
		case RECTANGLE_SHAPE: {
			Ref<SGRectangleShape2D> shape = node->get_shape();

			handles.resize(3);
			Vector2 ext = shape->get_extents()->to_float();
			handles.write[0] = Point2(ext.x, 0);
			handles.write[1] = Point2(0, -ext.y);
			handles.write[2] = Point2(ext.x, -ext.y);

			p_overlay->draw_texture(h, gt.xform(handles[0]) - size);
			p_overlay->draw_texture(h, gt.xform(handles[1]) - size);
			p_overlay->draw_texture(h, gt.xform(handles[2]) - size);
		} break;

		case CIRCLE_SHAPE: {
			Ref<SGCircleShape2D> shape = node->get_shape();

			handles.resize(1);
			handles.write[0] = Point2(fixed(shape->get_radius()).to_float(), 0);

			p_overlay->draw_texture(h, gt.xform(handles[0]) - size);
		} break;

		case CAPSULE_SHAPE: {
			Ref<SGCapsuleShape2D> shape = node->get_shape();

			handles.resize(2);
			float height = fixed(shape->get_height()).to_float();
			float radius = fixed(shape->get_radius()).to_float();
			handles.write[0] = Point2(radius, 0);
			handles.write[1] = Point2(0, -(height / 2 + radius));

			p_overlay->draw_texture(h, gt.xform(handles[0]) - size);
			p_overlay->draw_texture(h, gt.xform(handles[1]) - size);
		} break;
	}
}

void SGCollisionShape2DEditor::edit(Node *p_node) {
	if (!canvas_item_editor) {
		canvas_item_editor = CanvasItemEditor::get_singleton();
	}

	if (p_node) {
		node = Object::cast_to<SGCollisionShape2D>(p_node);

		_get_current_shape_type();
	}
	else {
		edit_handle = -1;
		shape_type = -1;

		node = nullptr;
	}

	canvas_item_editor->update_viewport();
}

SGCollisionShape2DEditor::SGCollisionShape2DEditor(EditorNode *p_editor) {
	node = nullptr;
	canvas_item_editor = nullptr;
	editor = p_editor;

	undo_redo = p_editor->get_undo_redo();

	edit_handle = -1;
	pressed = false;
}

bool SGCollisionShape2DEditorPlugin::handles(Object *p_obj) const {
	SGCollisionShape2D *node = Object::cast_to<SGCollisionShape2D>(p_obj);
	return (bool)node;
}

void SGCollisionShape2DEditorPlugin::edit(Object *p_obj) {
	collision_shape2d_editor->edit(Object::cast_to<Node>(p_obj));
}

void SGCollisionShape2DEditorPlugin::make_visible(bool visible) {
	if (!visible) {
		edit(nullptr);
	}
}

SGCollisionShape2DEditorPlugin::SGCollisionShape2DEditorPlugin(EditorNode *p_editor)
{
	editor = p_editor;
	collision_shape2d_editor = memnew(SGCollisionShape2DEditor(p_editor));
	p_editor->get_gui_base()->add_child(collision_shape2d_editor);
}

SGCollisionShape2DEditorPlugin::~SGCollisionShape2DEditorPlugin() {

}
