/*************************************************************************/
/*  animation_retargeting_editor_plugin.cpp                              */
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

#include "animation_retargeting_editor_plugin.h"

#include "modules/animation_retargeting/animation_retargeting.h"

/**
 * @author Smix8
 */

void AnimationRetargetingEditorPlugin::_notification(int p_what) {
}

void AnimationRetargetingEditorPlugin::_calculate_retargeting_data() {
	if (!animation_retargeting) {
		return;
	}

	if (scene2d_calculate_retargeting_data_button->is_pressed() || scene3d_calculate_retargeting_data_button->is_pressed()) {
		animation_retargeting->calculate_retargeting_data();
	}
	if (!(animation_retargeting->has_retargeting_data())) {
		scene2d_start_retargeting_button->set_disabled(true);
		scene3d_start_retargeting_button->set_disabled(true);
	};
	if (animation_retargeting->has_retargeting_data()) {
		scene2d_start_retargeting_button->set_disabled(false);
		scene3d_start_retargeting_button->set_disabled(false);
	};
}

void AnimationRetargetingEditorPlugin::_start_retargeting() {
	if (!animation_retargeting) {
		return;
	}

	if (scene3d_start_retargeting_button->is_pressed() || scene2d_start_retargeting_button->is_pressed()) {
		animation_retargeting->start_retargeting();
	}
}

void AnimationRetargetingEditorPlugin::_toggle_correction_mode() {
	if (!animation_retargeting) {
		return;
	};
	

	if (scene3d_correction_mode_button->is_pressed()) {
		animation_retargeting->enable_correction_mode();
		scene2d_correction_mode_button->set_pressed(false);
	};
	if (!scene3d_correction_mode_button->is_pressed()) {
		animation_retargeting->disable_correction_mode();
		scene3d_correction_mode_button->set_pressed(false);
		scene2d_correction_mode_button->set_pressed(false);
	};
}


void AnimationRetargetingEditorPlugin::edit(Object *p_object) {
	if (p_object != animation_retargeting) {
		if (animation_retargeting) {
			scene2d_calculate_retargeting_data_button->set_pressed(false);
			scene2d_start_retargeting_button->set_pressed(false);
			scene2d_correction_mode_button->set_pressed(false);

			scene3d_calculate_retargeting_data_button->set_pressed(false);
			scene3d_start_retargeting_button->set_pressed(false);
			scene3d_correction_mode_button->set_pressed(false);

			_start_retargeting();
		}
	}

	AnimationRetargeting *s = Object::cast_to<AnimationRetargeting>(p_object);
	if (!s) {
		return;
	}

	animation_retargeting = s;
}

bool AnimationRetargetingEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("AnimationRetargeting");
}

void AnimationRetargetingEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		
		scene2d_calculate_retargeting_data_button->show();
		scene2d_start_retargeting_button->show();
		scene2d_correction_mode_button->show();

		scene3d_calculate_retargeting_data_button->show();
		scene3d_start_retargeting_button->show();
		scene3d_correction_mode_button->show();
	} else {
		scene2d_calculate_retargeting_data_button->hide();
		scene2d_start_retargeting_button->hide();
		scene2d_correction_mode_button->hide();

		scene3d_calculate_retargeting_data_button->hide();
		scene3d_start_retargeting_button->hide();
		scene3d_correction_mode_button->hide();
	}
}

void AnimationRetargetingEditorPlugin::_bind_methods() {
}

AnimationRetargetingEditorPlugin::AnimationRetargetingEditorPlugin(EditorNode *p_node) {
	editor = p_node;

	// 2D Editor Window

	scene2d_calculate_retargeting_data_button = memnew(Button);
	scene2d_calculate_retargeting_data_button->set_icon(editor->get_gui_base()->get_theme_icon("Play", "EditorIcons"));
	scene2d_calculate_retargeting_data_button->set_text(TTR("Calculate Retargeting Data"));
	scene2d_calculate_retargeting_data_button->hide();
	scene2d_calculate_retargeting_data_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_calculate_retargeting_data));
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_calculate_retargeting_data_button);

	scene2d_start_retargeting_button = memnew(Button);
	scene2d_start_retargeting_button->set_icon(editor->get_gui_base()->get_theme_icon("Play", "EditorIcons"));
	scene2d_start_retargeting_button->set_text(TTR("Retarget Animations"));	
	scene2d_start_retargeting_button->hide();
	scene2d_start_retargeting_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_start_retargeting));
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_start_retargeting_button);

	scene2d_correction_mode_button = memnew(Button);
	scene2d_correction_mode_button->set_icon(editor->get_gui_base()->get_theme_icon("Tools", "EditorIcons"));
	scene2d_correction_mode_button->set_text(TTR("Correction Mode"));
	scene2d_correction_mode_button->set_toggle_mode(true);
	scene2d_correction_mode_button->hide();
	scene2d_correction_mode_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_toggle_correction_mode));
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_correction_mode_button);

	// 3D Editor Window

	scene3d_calculate_retargeting_data_button = memnew(Button);
	scene3d_calculate_retargeting_data_button->set_icon(editor->get_gui_base()->get_theme_icon("Play", "EditorIcons"));
	scene3d_calculate_retargeting_data_button->set_text(TTR("Calculate Retargeting Data"));
	scene3d_calculate_retargeting_data_button->hide();
	scene3d_calculate_retargeting_data_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_calculate_retargeting_data));
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_calculate_retargeting_data_button);

	scene3d_start_retargeting_button = memnew(Button);
	scene3d_start_retargeting_button->set_icon(editor->get_gui_base()->get_theme_icon("Play", "EditorIcons"));
	scene3d_start_retargeting_button->set_text(TTR("Retarget Animations"));
	scene3d_start_retargeting_button->hide();
	scene3d_start_retargeting_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_start_retargeting));
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_start_retargeting_button);

	scene3d_correction_mode_button = memnew(Button);
	scene3d_correction_mode_button->set_icon(editor->get_gui_base()->get_theme_icon("Tools", "EditorIcons"));
	scene3d_correction_mode_button->set_text(TTR("Correction Mode"));
	scene3d_correction_mode_button->set_toggle_mode(true);
	scene3d_correction_mode_button->hide();
	scene3d_correction_mode_button->connect("pressed", callable_mp(this, &AnimationRetargetingEditorPlugin::_toggle_correction_mode));
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_correction_mode_button);

	animation_retargeting = nullptr;
}

AnimationRetargetingEditorPlugin::~AnimationRetargetingEditorPlugin() {
	// 2D Editor Window

	if (scene2d_calculate_retargeting_data_button != nullptr) {
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_calculate_retargeting_data_button);
	}
	if (scene2d_start_retargeting_button != nullptr) {
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_start_retargeting_button);
	}
	if (scene2d_correction_mode_button != nullptr) {
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_correction_mode_button);
	}

	// 3D Editor Window

	if (scene3d_calculate_retargeting_data_button != nullptr) {
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_calculate_retargeting_data_button);
	}
	if (scene3d_start_retargeting_button != nullptr) {
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_start_retargeting_button);
	}
	if (scene3d_correction_mode_button != nullptr) {
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_correction_mode_button);
	}
}
