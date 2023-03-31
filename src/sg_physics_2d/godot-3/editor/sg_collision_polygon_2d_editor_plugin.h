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

#ifndef SG_COLLISION_POLYGON_2D_EDITOR_PLUGIN_H
#define SG_COLLISION_POLYGON_2D_EDITOR_PLUGIN_H

#include <editor/plugins/abstract_polygon_2d_editor.h>

#include "../scene/2d/sg_collision_polygon_2d.h"

class SGCollisionPolygon2DEditor : public AbstractPolygon2DEditor {

	GDCLASS(SGCollisionPolygon2DEditor, AbstractPolygon2DEditor);

	SGCollisionPolygon2D *node;

protected:
	virtual Node2D *_get_node() const;
	virtual void _set_node(Node *p_polygon);

public:
	SGCollisionPolygon2DEditor(EditorNode *p_editor);
};

class SGCollisionPolygon2DEditorPlugin : public AbstractPolygon2DEditorPlugin {

	GDCLASS(SGCollisionPolygon2DEditorPlugin, AbstractPolygon2DEditorPlugin);

public:
	SGCollisionPolygon2DEditorPlugin(EditorNode *p_node);
};

#endif
