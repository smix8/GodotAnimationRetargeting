/*************************************************************************/
/*  animation_retargeting.h                                              */
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

#ifndef ANIMATION_RETARGETING_H
#define ANIMATION_RETARGETING_H


#include "core/math/transform.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "scene/animation/animation_player.h"
#include "scene/3d/skeleton.h"
#include "scene/resources/packed_scene.h"
/**
 * @author Smix8
 */

class AnimationRetargeting : public Node {
	GDCLASS(AnimationRetargeting, Node);

	public:

		// EXPORT

		enum AnimationExportFormat {
			ANIMATION_EXPORT_TRES,
			ANIMATION_EXPORT_ANIM
		};
		
		AnimationExportFormat animation_export_format = ANIMATION_EXPORT_ANIM;

		String animation_export_directory = "res://";
		bool export_animations = true;
		bool replace_retarget_animations = true;
		bool export_animationplayer = true;

		String animation_rename_prefix = "";
		String animation_rename_suffix = "";

		// RIG MAPPING

		enum SourceRigType {
			SOURCE_RIG_CUSTOM,
			SOURCE_RIG_RIGIFY2,
			SOURCE_RIG_GENESIS3AND8,
			SOURCE_RIG_3DSMAX,
			SOURCE_RIG_MAKEHUMAN
		};
		enum TargetRigType {
			TARGET_RIG_CUSTOM,
			TARGET_RIG_RIGIFY2,
			TARGET_RIG_GENESIS3AND8,
			TARGET_RIG_3DSMAX,
			TARGET_RIG_MAKEHUMAN
		};

		SourceRigType source_rig_type = SOURCE_RIG_CUSTOM;
		TargetRigType target_rig_type = TARGET_RIG_CUSTOM;

		// RETARGETING

		Dictionary retarget_mapping;

		Array ignore_bones;
		
		bool retarget_position = false;
		bool retarget_rotation = true;
		bool retarget_scale = false;
		bool root_motion = false;
		bool fixate_in_place = false;
		bool sync_playback = false;
		double source_skeleton_scale = 1.0;
		double retarget_skeleton_scale = 1.0;

		NodePath source_skeleton_node_path;
		NodePath source_animationplayer_node_path;
		NodePath retarget_skeleton_node_path;
		NodePath retarget_animationplayer_node_path;

		void set_source_skeleton_scale(const float &p_source_skeleton_scale);
		float get_source_skeleton_scale();
		void set_retarget_skeleton_scale(const float &p_retarget_skeleton_scale);
		float get_retarget_skeleton_scale();

		Skeleton *_source_skeleton = nullptr;
		AnimationPlayer *_source_animationplayer = nullptr;
		Skeleton *_retarget_skeleton = nullptr;
		AnimationPlayer *_retarget_animationplayer = nullptr;

		enum AnimationRetargetMode {
			RETARGET_MODE_ANIMATIONPLAYER,
			RETARGET_MODE_CURRENT_ANIMATION,
			RETARGET_MODE_LIVE_MOTION_CAPTURE,
			RETARGET_MODE_NEW_SOURCE_ANIMATION,
			RETARGET_MODE_EXISTING_TARGET_ANIMATION,
		};

		AnimationRetargetMode retarget_mode = RETARGET_MODE_ANIMATIONPLAYER;
				
		void set_retarget_mode(AnimationRetargetMode p_retarget_mode);
		AnimationRetargetMode get_retarget_mode() const;

		bool calculate_retargeting_data();
		void start_retargeting();

		bool has_retargeting_data() const;

		void set_export_animations(const bool &p_enabled);
		bool get_export_animations();

		void retarget_skeleton_animations();

		void set_animation_rename_prefix(const String &p_prefix);
		String get_animation_rename_prefix() const;
		void set_animation_rename_suffix(const String &p_suffix);
		String get_animation_rename_suffix() const;

		void set_replace_retarget_animations(const bool &p_enabled);
		bool get_replace_retarget_animations();

		void set_export_animationplayer(const bool &p_enabled);
		bool get_export_animationplayer();

		void set_retarget_position(const bool &p_enabled);
		bool get_retarget_position();
		void set_retarget_rotation(const bool &p_enabled);
		bool get_retarget_rotation();
		void set_retarget_scale(const bool &p_enabled);
		bool get_retarget_scale();
		void set_root_motion(const bool &p_enabled);
		bool get_root_motion();
		void set_fixate_in_place(const bool &p_enabled);
		bool get_fixate_in_place();
		void set_sync_playback(const bool &p_enabled);
		bool get_sync_playback();

		// CORRECTION

		enum CorrectionMode {
			CORRECTION_MODE_DISABLED,
			CORRECTION_MODE_ENABLED
		};

		bool correction_mode = CORRECTION_MODE_DISABLED;
		void enable_correction_mode();
		void disable_correction_mode();

		void set_source_skeleton_path(const NodePath &p_source_skeleton_node_path);
		NodePath get_source_skeleton_path();
		void set_retarget_skeleton_path(const NodePath &p_retarget_skeleton_node_path);
		NodePath get_retarget_skeleton_path();
		void set_source_animationplayer_path(const NodePath &p_node);
		NodePath get_source_animationplayer_path();
		void set_retarget_animationplayer_path(const NodePath &p_node);
		NodePath get_retarget_animationplayer_path();

		Skeleton *get_source_skeleton() const { return _source_skeleton; };
		Skeleton *get_retarget_skeleton() const { return _retarget_skeleton; };
		AnimationPlayer *get_source_animationplayer() const { return _source_animationplayer; };
		AnimationPlayer *get_retarget_animationplayer() const { return _retarget_animationplayer; };

		void set_animation_export_format(AnimationExportFormat p_animation_export_format);
		AnimationExportFormat get_animation_export_format() const;

		void set_export_directory(const String &p_directory);
		String get_export_directory() const;

		void set_source_rig_type(SourceRigType p_source_rig_type);
		SourceRigType get_source_rig_type() const;
		void set_target_rig_type(TargetRigType p_target_rig_type);
		TargetRigType get_target_rig_type() const;


		String correction_bone = "";
		Vector3 position_correction = Vector3(0.0, 0.0, 0.0);
		Vector3 rotation_correction = Vector3(0.0, 0.0, 0.0);
		Vector3 scale_correction = Vector3(0.0, 0.0, 0.0);

		void set_correction_bone(const String &p_correction_bone);
		String get_correction_bone() const;
		void set_position_correction(const Vector3 &p_position_correction);
		Vector3 get_position_correction() const;
		void set_rotation_correction(const Vector3 &p_rotation_correction);
		Vector3 get_rotation_correction() const;
		void set_scale_correction(const Vector3 &p_scale_correction);
		Vector3 get_scale_correction() const;

		void set_ignore_bones(const Array &p_ignore_bones);
		Array get_ignore_bones();
		void set_custom_bone_mapping(const Dictionary &p_custom_bone_mapping);
		Dictionary get_custom_bone_mapping();

		AnimationRetargeting();
		virtual ~AnimationRetargeting();

	protected:
		virtual void
		_validate_property(PropertyInfo &property) const;
		static void _bind_methods();

	private:

		int _source_root_bone_inx = -1;
		StringName _source_root_bone_name = "";
		int _retarget_root_bone_inx = -1;
		StringName _retarget_root_bone_name = "";
		double _skeleton_scale_mod = 1.0;
		double _root_motion_scale = 1.0;
		String _source_path_animationplayer_to_skeleton = "";
		String _retarget_path_animationplayer_to_skeleton = "";
		NodePath _source_animationplayer_root_nodepath = NodePath();
		NodePath _retarget_animationplayer_root_nodepath = NodePath();
		Node* _source_animationplayer_root_node;
		Node* _retarget_animationplayer_root_node;
		bool _calculate_retargeting_data = true;

		bool _valid_setup();
		Ref<Animation> _retarget_animation_track(Ref<Animation> &p_source_animation);
		void _add_missing_bones_in_animation_track(Ref<Animation> &p_new_retargeted_animation);

		struct CorrectionBoneData {
			Vector3 position_correction;
			Vector3 rotation_correction;
			Vector3 scale_correction;
			CorrectionBoneData(){
				position_correction = Vector3(0.0, 0.0, 0.0);
				rotation_correction = Vector3(0.0, 0.0, 0.0);
				scale_correction = Vector3(0.0, 0.0, 0.0);
			};
		};

		Map<StringName, CorrectionBoneData> correction_bone_mapping;

		Dictionary custom_bone_mapping;
		Dictionary mixamo_mapping;
		Dictionary rigify2_bone_mapping;
		Dictionary genesis3and8_bone_mapping;
		Dictionary max3ds_bone_mapping;
		Dictionary makehuman_bone_mapping;

};

VARIANT_ENUM_CAST(AnimationRetargeting::AnimationRetargetMode);
VARIANT_ENUM_CAST(AnimationRetargeting::AnimationExportFormat);
VARIANT_ENUM_CAST(AnimationRetargeting::SourceRigType);
VARIANT_ENUM_CAST(AnimationRetargeting::TargetRigType);
VARIANT_ENUM_CAST(AnimationRetargeting::CorrectionMode);


#endif // ANIMATION_RETARGETING_H
