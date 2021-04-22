/*************************************************************************/
/*  animation_retargeting_editor_plugin.h                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef ANIMATION_RETARGETING_EDITOR_PLUGIN_H
#define ANIMATION_RETARGETING_EDITOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"

/**
 * @author Smix8
 */

class AnimationRetargeting;

class AnimationRetargetingEditorPlugin : public EditorPlugin {
	GDCLASS(AnimationRetargetingEditorPlugin, EditorPlugin);

	AnimationRetargeting *animation_retargeting;

	Button *scene2d_calculate_retargeting_data_button = nullptr;
	Button *scene2d_start_retargeting_button = nullptr;
	Button *scene2d_correction_mode_button = nullptr;

	Button *scene3d_calculate_retargeting_data_button = nullptr;
	Button *scene3d_start_retargeting_button = nullptr;
	Button *scene3d_correction_mode_button = nullptr;

	EditorNode *editor;

public:
	virtual String get_name() const override { return "AnimationRetargeting"; }
	bool has_main_screen() const override { return false; }
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;
	virtual void make_visible(bool p_visible) override;

	AnimationRetargetingEditorPlugin(EditorNode *p_node);
	~AnimationRetargetingEditorPlugin();

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	void _start_retargeting();
	void _toggle_correction_mode();
	void _calculate_retargeting_data();
};

#endif // ANIMATION_RETARGETING_EDITOR_PLUGIN_H
