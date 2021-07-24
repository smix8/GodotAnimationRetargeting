/*************************************************************************/
/*  animation_retargeting.cpp											 */
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

#include "animation_retargeting.h"

/**
 * @author Smix8
 */

bool AnimationRetargeting::has_retargeting_data() const {
	if (_source_animationplayer->get_root() != _source_animationplayer_root_nodepath) {
		return false;
	}
	if (_retarget_animationplayer->get_root() != _retarget_animationplayer_root_nodepath) {
		return false;
	}

	return !retarget_mapping.empty() && !_calculate_retargeting_data;
}

bool AnimationRetargeting::_valid_setup() {
	_source_skeleton = Object::cast_to<Skeleton>(get_node(source_skeleton_node_path));
	_source_animationplayer = Object::cast_to<AnimationPlayer>(get_node(source_animationplayer_node_path));
	_retarget_skeleton = Object::cast_to<Skeleton>(get_node(retarget_skeleton_node_path));
	_retarget_animationplayer = Object::cast_to<AnimationPlayer>(get_node(retarget_animationplayer_node_path));

	ERR_FAIL_COND_V_MSG(_source_skeleton == nullptr, false, vformat("Missing Source Skeleton, did you pick the wrong nodepath?"));
	ERR_FAIL_COND_V_MSG(_source_animationplayer == nullptr, false, vformat("Missing Source AnimationPlayer, did you pick the wrong nodepath?"));
	ERR_FAIL_COND_V_MSG(_retarget_skeleton == nullptr, false, vformat("Missing Retarget Skeleton, did you pick the wrong nodepath?"));

	if (_source_skeleton == nullptr || _source_animationplayer == nullptr || _retarget_skeleton == nullptr) {
		retarget_mapping.clear();
		return false;
	}
	if (retarget_mode == AnimationRetargeting::RETARGET_MODE_CURRENT_ANIMATION) {
		String current_animation_playback_id = _source_animationplayer->get_assigned_animation();
		if (current_animation_playback_id == "") {
			ERR_FAIL_COND_V_MSG(true, false, vformat("Failed to get assigned Animation from AnimationPlayer. AnimationTrack must be loaded in Editor or Animation needs to play currently."));
			return false;
		}
	}
	return true;
}

void AnimationRetargeting::start_retargeting() {
	if (!_valid_setup()) {
		return;
	}

	for (int bone_inx = 0; bone_inx < _source_skeleton->get_bone_count(); bone_inx++) {
		_source_skeleton->set_bone_custom_pose(bone_inx, Transform());
		_source_skeleton->set_bone_pose(bone_inx, Transform());
	}

	for (int bone_inx = 0; bone_inx < _retarget_skeleton->get_bone_count(); bone_inx++) {
		_retarget_skeleton->set_bone_custom_pose(bone_inx, Transform());
		_retarget_skeleton->set_bone_pose(bone_inx, Transform());
	}

	String current_animation_playback_id = _source_animationplayer->get_assigned_animation();

	if (calculate_retargeting_data()) {
		retarget_skeleton_animations();
	};

	if (_source_animationplayer->has_animation(current_animation_playback_id)) {
		if (sync_playback) {
			_source_animationplayer->stop();
		}
		_source_animationplayer->play(current_animation_playback_id);
	}
	if (_retarget_animationplayer->has_animation(current_animation_playback_id)) {
		if (sync_playback) {
			_retarget_animationplayer->stop();
		}
		_retarget_animationplayer->play(current_animation_playback_id);
	}
}

bool AnimationRetargeting::calculate_retargeting_data() {
	if (!_valid_setup()) {
		return false;
	}
	if (has_retargeting_data()) {
		return true;
	}
	
	_source_animationplayer_root_nodepath = _source_animationplayer->get_root();
	_retarget_animationplayer_root_nodepath = _retarget_animationplayer->get_root();
	_source_animationplayer_root_node = _source_animationplayer->get_node(_source_animationplayer_root_nodepath);
	_retarget_animationplayer_root_node = _retarget_animationplayer->get_node(_retarget_animationplayer_root_nodepath);
	_source_path_animationplayer_to_skeleton = String(_source_animationplayer_root_node->get_path_to(_source_skeleton));
	_retarget_path_animationplayer_to_skeleton = String(_retarget_animationplayer_root_node->get_path_to(_retarget_skeleton));

	for (int source_skeleton_bone_inx = 0; source_skeleton_bone_inx < _source_skeleton->get_bone_count(); source_skeleton_bone_inx++) {

		StringName bone_name = _source_skeleton->get_bone_name(source_skeleton_bone_inx);

		if (_source_skeleton->get_bone_parent(source_skeleton_bone_inx) == -1) {
			_source_root_bone_inx = source_skeleton_bone_inx;
			_source_root_bone_name = bone_name;
		}

		if (source_rig_type == AnimationRetargeting::SOURCE_RIG_CUSTOM && target_rig_type == AnimationRetargeting::TARGET_RIG_CUSTOM && !custom_bone_mapping.empty()) {
			if (custom_bone_mapping.has(bone_name)) {
				bone_name = custom_bone_mapping.get(bone_name, StringName());
			} else {
				custom_bone_mapping[bone_name] = "";
			}
		}

		int retarget_skeleton_bone_inx = _retarget_skeleton->find_bone(bone_name);
		if (retarget_skeleton_bone_inx < 0) {
			continue;
		}
		if (_retarget_skeleton->get_bone_parent(retarget_skeleton_bone_inx) == -1) {
			_retarget_root_bone_inx = retarget_skeleton_bone_inx;
			_retarget_root_bone_name = bone_name;
		}

		Transform _source_bone_rest = _source_skeleton->get_bone_rest(source_skeleton_bone_inx);
		Transform _target_bone_rest = _retarget_skeleton->get_bone_rest(retarget_skeleton_bone_inx);

		Vector3 _source_bone_rest_pos = _source_bone_rest.origin;
		Vector3 _target_bone_rest_pos = _target_bone_rest.origin;

		Quat _source_bone_rest_rot = _source_bone_rest.basis.get_rotation_quat();
		Quat _target_bone_rest_rot = _target_bone_rest.basis.get_rotation_quat();

		Vector3 position_offset_vector;

		_root_motion_scale = source_skeleton_scale / retarget_skeleton_scale;

		if (retarget_skeleton_scale > source_skeleton_scale) {
			_skeleton_scale_mod = retarget_skeleton_scale / source_skeleton_scale;
			position_offset_vector = (_target_bone_rest_pos) - ((_source_bone_rest_pos * retarget_skeleton_scale) / _skeleton_scale_mod);
		} else if (retarget_skeleton_scale < source_skeleton_scale) {
			_skeleton_scale_mod = _root_motion_scale;
			position_offset_vector = (_target_bone_rest_pos) - ((_source_bone_rest_pos * retarget_skeleton_scale) * _skeleton_scale_mod);
		} else {
			_root_motion_scale = 1.0;
			_skeleton_scale_mod = 1.0;
			position_offset_vector = _target_bone_rest_pos - _source_bone_rest_pos;
		}

		Quat rotation_offset_quat = _target_bone_rest_rot.normalized().inverse() * _source_bone_rest_rot.normalized();
		Vector3 scale_offset_vector = _source_bone_rest.basis.get_scale() - _target_bone_rest.basis.get_scale();

		Dictionary od;
		od["origin_offset"] = position_offset_vector;
		od["quat_offset"] = rotation_offset_quat;
		od["scale_offset"] = scale_offset_vector;
		retarget_mapping[bone_name] = od;
	}

	set_custom_bone_mapping(custom_bone_mapping);

	_calculate_retargeting_data = false;

	if (retarget_mapping.empty()) {
		return false;
	}
	return true;
}

void AnimationRetargeting::retarget_skeleton_animations() {
	if (!_valid_setup()) {
		return;
	}

	String export_format_string = ".tres";
	if (animation_export_format == AnimationRetargeting::ANIMATION_EXPORT_ANIM) {
		export_format_string = ".anim";
	}

	AnimationPlayer *new_animationplayer = nullptr;
	if (export_animationplayer) {
		new_animationplayer = memnew(AnimationPlayer);
	}

	List<StringName> source_animation_list;
	List<StringName> retarget_animation_list;

	String playing_source_animation_id = _source_animationplayer->get_assigned_animation();
	String playing_retarget_animation_id = _retarget_animationplayer->get_assigned_animation();

	_source_animationplayer->get_animation_list(&source_animation_list);
	_retarget_animationplayer->get_animation_list(&retarget_animation_list);

	if (source_animation_list.size() > 0) {
		for (List<StringName>::Element *E = source_animation_list.front(); E; E = E->next()) {

			StringName animation_id = StringName(E->get());

			if (!(_source_animationplayer->has_animation(animation_id))) {
				continue;
			}
			
			if (retarget_mode == AnimationRetargeting::RETARGET_MODE_CURRENT_ANIMATION) {
				if (playing_source_animation_id != animation_id) {
					continue;
				}
			} else if (retarget_mode == AnimationRetargeting::RETARGET_MODE_NEW_SOURCE_ANIMATION) {
				if (_retarget_animationplayer->has_animation(animation_id)) {
					continue;
				}
			} else if (retarget_mode == AnimationRetargeting::RETARGET_MODE_EXISTING_TARGET_ANIMATION) {
				if (!(_retarget_animationplayer->has_animation(animation_id))) {
					continue;
				}
			}

			Ref<Animation> source_animation = _source_animationplayer->get_animation(animation_id);
	
			Ref<Animation> retargeted_animation = _retarget_animation_track(source_animation);

			StringName new_retargeted_animation_name = animation_rename_prefix + animation_id + animation_rename_suffix;
		
			if (replace_retarget_animations && _retarget_animationplayer->has_animation(new_retargeted_animation_name)) {
				_retarget_animationplayer->remove_animation(new_retargeted_animation_name);
			}

			if (!(_retarget_animationplayer->has_animation(new_retargeted_animation_name))) {
				_retarget_animationplayer->add_animation(new_retargeted_animation_name, retargeted_animation);
			}

			if (export_animations) {
				String save_animation_path = animation_export_directory + "/" + new_retargeted_animation_name + export_format_string;
				Error err = ResourceSaver::save(save_animation_path, retargeted_animation);
				ERR_FAIL_COND_MSG(err != OK, vformat("failed to save retargeted animation to export folder %s", save_animation_path));
			}

			if (export_animationplayer) {
				new_animationplayer->add_animation(new_retargeted_animation_name, retargeted_animation);
			}
		}

		if (export_animationplayer) {
			Ref<PackedScene> new_packed_scene = memnew(PackedScene);
			new_packed_scene->pack(new_animationplayer);
			String save_animationplayer_path = animation_export_directory + "/" + animation_rename_prefix + "AnimationPlayer" + animation_rename_suffix + ".scn";
			Error err = ResourceSaver::save(save_animationplayer_path, new_packed_scene);
			ERR_FAIL_COND_MSG(err != OK, vformat("failed to save retargeted animationplayer to export folder %s", save_animationplayer_path));
			memdelete(new_animationplayer);
		}
	}

	if (_source_animationplayer->has_animation(playing_source_animation_id)) {
		_source_animationplayer->play(playing_source_animation_id);
	}
	if (_retarget_animationplayer->has_animation(playing_retarget_animation_id)) {
		_retarget_animationplayer->play(playing_retarget_animation_id);
	}
}

void AnimationRetargeting::_add_missing_bones_in_animation_track(Ref<Animation> &p_new_retargeted_animation) {
	if (!_valid_setup()) {
		return;
	}

	String retarget_skeleton_string_path = "";

	if (_source_path_animationplayer_to_skeleton != _retarget_path_animationplayer_to_skeleton) {
		for (int _track_inx = 0; _track_inx < p_new_retargeted_animation->get_track_count(); _track_inx++) {
			retarget_skeleton_string_path = p_new_retargeted_animation->track_get_path(_track_inx);
			if (retarget_skeleton_string_path.begins_with(_source_path_animationplayer_to_skeleton)) {
				retarget_skeleton_string_path = retarget_skeleton_string_path.replace(_source_path_animationplayer_to_skeleton, _retarget_path_animationplayer_to_skeleton);
				int subname_count = p_new_retargeted_animation->track_get_path(_track_inx).get_subname_count();
				if (subname_count > 0) {
					String bone_name = p_new_retargeted_animation->track_get_path(_track_inx).get_subname(subname_count - 1);
					if (target_rig_type == AnimationRetargeting::TARGET_RIG_CUSTOM && !custom_bone_mapping.empty()) {
						if (custom_bone_mapping.has(bone_name)) {
							String _bone_remapped = custom_bone_mapping.get(bone_name, String());
							if (_bone_remapped == "") {
								_bone_remapped = "bone_missing_mapping";
							}
							retarget_skeleton_string_path = retarget_skeleton_string_path.replace(bone_name, _bone_remapped);
						}
					}
				}
				p_new_retargeted_animation->track_set_path(_track_inx, NodePath(retarget_skeleton_string_path));
			}
		}
	}

	Array bone_keys = retarget_mapping.keys();

	if (target_rig_type == AnimationRetargeting::TARGET_RIG_CUSTOM && !custom_bone_mapping.empty()) {
		bone_keys = custom_bone_mapping.values();
	}
	if (target_rig_type == AnimationRetargeting::TARGET_RIG_GENESIS3AND8) {
		bone_keys = genesis3and8_bone_mapping.keys();
	}

	bool found_bone_track = false;
	String _searching_bone_name = "";

	for (int bone_key_inx = 0; bone_key_inx < bone_keys.size(); bone_key_inx++) {

		_searching_bone_name = bone_keys[bone_key_inx];
		if (_searching_bone_name == "") {
			continue;
		}

		found_bone_track = false;
		
		for (int _track_inx = 0; _track_inx < p_new_retargeted_animation->get_track_count(); _track_inx++) {
			if (!p_new_retargeted_animation->track_get_type(_track_inx) == Animation::TYPE_TRANSFORM) {
				continue;
			}
			int subname_count = p_new_retargeted_animation->track_get_path(_track_inx).get_subname_count();
			if (subname_count < 1) {
				continue;
			}
			String bone_name = p_new_retargeted_animation->track_get_path(_track_inx).get_subname(subname_count - 1);

			if (bone_name == _searching_bone_name) {
				found_bone_track = true;
				retarget_skeleton_string_path = p_new_retargeted_animation->track_get_path(_track_inx);
				if (retarget_skeleton_string_path.begins_with(_source_path_animationplayer_to_skeleton)) {
					retarget_skeleton_string_path = retarget_skeleton_string_path.replace(_source_path_animationplayer_to_skeleton, _retarget_path_animationplayer_to_skeleton);
					p_new_retargeted_animation->track_set_path(_track_inx, NodePath(retarget_skeleton_string_path));
				}
				break;
			}
		}
		
		if (!found_bone_track) {
			int new_track_inx = p_new_retargeted_animation->get_track_count() - 1;
			if (new_track_inx > 0) {
				String bone_stringpath = _retarget_path_animationplayer_to_skeleton + ":" + _searching_bone_name;

				NodePath new_animation_track_bone_nodepath = NodePath(bone_stringpath);
				Vector3 new_translation = Vector3(0.0, 0.0, 0.0);
				Quat new_rotation_quat = Quat();
				Vector3 new_scale = Vector3(1.0, 1.0, 1.0);

				p_new_retargeted_animation->add_track(Animation::TYPE_TRANSFORM, new_track_inx);
				p_new_retargeted_animation->track_set_path(new_track_inx, new_animation_track_bone_nodepath);
				p_new_retargeted_animation->transform_track_insert_key(new_track_inx, 0.0, new_translation, new_rotation_quat, new_scale);
				p_new_retargeted_animation->transform_track_insert_key(new_track_inx, p_new_retargeted_animation->get_length(), new_translation, new_rotation_quat, new_scale);
			}
		}
	}

	// clean old transform tracks from source skeleton
	// can't delete in same loop due to index shift so one by one
	String _trackpath = "";
	bool _deleting_tracks = true;
	bool _deleted_track = false;
	while (_deleting_tracks) {
		_deleted_track = false;
		for (int _track_inx = 0; _track_inx < p_new_retargeted_animation->get_track_count(); _track_inx++) {
			if (_deleted_track == true) {
				continue;
			}
			if (!p_new_retargeted_animation->track_get_type(_track_inx) == Animation::TYPE_TRANSFORM) {
				continue;
			}
			_trackpath = String(p_new_retargeted_animation->track_get_path(_track_inx));
			if (!_trackpath.begins_with(_retarget_path_animationplayer_to_skeleton)) {
				p_new_retargeted_animation->remove_track(_track_inx);
				_deleted_track = true;
			} else if (_trackpath.find("bone_missing_mapping") != -1) {
				p_new_retargeted_animation->remove_track(_track_inx);
				_deleted_track = true;
			}
		}
		_deleting_tracks = _deleted_track;
	}
}

Ref<Animation> AnimationRetargeting::_retarget_animation_track(Ref<Animation> &p_source_animation) {
	ERR_FAIL_COND_V_MSG(_source_skeleton == nullptr, p_source_animation, vformat("Missing Source Skeleton, did you pick the wrong nodepath?"));
	ERR_FAIL_COND_V_MSG(_source_animationplayer == nullptr, p_source_animation, vformat("Missing Source AnimationPlayer, did you pick the wrong nodepath?"));
	ERR_FAIL_COND_V_MSG(_retarget_skeleton == nullptr, p_source_animation, vformat("Missing Retarget Skeleton, did you pick the wrong nodepath?"));
	ERR_FAIL_COND_V_MSG(retarget_mapping.empty(), p_source_animation, vformat("Missing retargeting data"));
	ERR_FAIL_COND_V_MSG(p_source_animation == nullptr, p_source_animation, vformat("Missing valid animation for retargeting"));

	Ref<Animation> new_retargeted_animation = memnew(Animation);

	List<PropertyInfo> plist;
	p_source_animation->get_property_list(&plist);

	for (List<PropertyInfo>::Element *E = plist.front(); E; E = E->next()) {
		if (E->get().usage & PROPERTY_USAGE_STORAGE) {
			new_retargeted_animation->set(E->get().name, p_source_animation->get(E->get().name));
		}
	}
	new_retargeted_animation->set_path("");

	_add_missing_bones_in_animation_track(new_retargeted_animation);

	bool apply_position_correction = (_retarget_skeleton->find_bone(correction_bone) != -1);

	for (int _track_inx = 0; _track_inx < new_retargeted_animation->get_track_count(); _track_inx++) {

		if (!new_retargeted_animation->track_get_type(_track_inx) == Animation::TYPE_TRANSFORM) {
			continue;
		}

		int subname_count = new_retargeted_animation->track_get_path(_track_inx).get_subname_count();
		StringName bone_name = new_retargeted_animation->track_get_path(_track_inx).get_subname(subname_count - 1);

		if (!retarget_mapping.has(bone_name)) {
			continue;
		}
		if (ignore_bones.has(bone_name)) {
			continue;
		}

		if ((target_rig_type == AnimationRetargeting::TARGET_RIG_CUSTOM) && !custom_bone_mapping.empty()) {
			if (!custom_bone_mapping.values().has(bone_name)) {
				continue;
			}
		} else if ((target_rig_type == AnimationRetargeting::TARGET_RIG_RIGIFY2) && !rigify2_bone_mapping.has(bone_name)) {
			continue;
		} else if ((target_rig_type == AnimationRetargeting::TARGET_RIG_GENESIS3AND8) && !genesis3and8_bone_mapping.has(bone_name)) {
			continue;
		} else if ((target_rig_type == AnimationRetargeting::TARGET_RIG_3DSMAX) && !max3ds_bone_mapping.has(bone_name)) {
			continue;
		} else if ((target_rig_type == AnimationRetargeting::TARGET_RIG_MAKEHUMAN) && !makehuman_bone_mapping.has(bone_name)) {
			continue;
		}
		
		for (int _track_key_inx = 0; _track_key_inx < new_retargeted_animation->track_get_key_count(_track_inx); _track_key_inx++) {
			
			Dictionary _keyframe_value_dict = new_retargeted_animation->track_get_key_value(_track_inx, _track_key_inx);

			Vector3 _key_origin = _keyframe_value_dict["location"];
			Quat _key_quat = _keyframe_value_dict["rotation"];
			Vector3 _key_scale = _keyframe_value_dict["scale"];

			Dictionary _bone_dict = retarget_mapping[bone_name];

			Vector3 _origin_offset = _bone_dict["origin_offset"];
			Quat _quat_offset = _bone_dict["quat_offset"];
			Vector3 _scale_offset = _bone_dict["scale_offset"];
						
			Vector3 _new_key_origin = _key_origin;
			Quat _new_key_quat = _key_quat;
			Vector3 _new_key_scale = _key_scale;

			// BONE SCALE
			if ((retarget_scale) || ((bone_name == _retarget_root_bone_name) && root_motion)) {
				if (bone_name == correction_bone) {
					_new_key_scale = _key_scale + _scale_offset + scale_correction;
				} else {
					_new_key_scale = _key_scale + _scale_offset;
				}
			}

			// BONE POSITION
			if ((retarget_position) || ((bone_name == _retarget_root_bone_name) && root_motion)) {
				if ((bone_name == _retarget_root_bone_name) && root_motion) {
					if (bone_name == correction_bone) {
						_new_key_origin = (_key_origin * _root_motion_scale) + _origin_offset + position_correction;
					} else {
						_new_key_origin = (_key_origin * _root_motion_scale) + _origin_offset;
					}
				} else {
					if (apply_position_correction && bone_name == correction_bone) {
						_new_key_origin = (_key_origin * _skeleton_scale_mod) + _origin_offset + position_correction;
					} else {
						_new_key_origin = (_key_origin * _skeleton_scale_mod) + _origin_offset;
					}
				}
			}

			// BONE ROTATION
			if ((retarget_rotation) || ((bone_name == _retarget_root_bone_name) && root_motion)) {
				if (bone_name == correction_bone) {
					Vector3 _rotation_correction_rad;
					_rotation_correction_rad.x = Math::deg2rad(rotation_correction.x);
					_rotation_correction_rad.y = Math::deg2rad(rotation_correction.y);
					_rotation_correction_rad.z = Math::deg2rad(rotation_correction.z);
					_new_key_quat = (((_quat_offset * _key_quat).normalized()) * Quat(_rotation_correction_rad).normalized()).normalized();		

				} else {
					_new_key_quat = (_quat_offset * _key_quat).normalized();
				}
			}

			if ((bone_name == _retarget_root_bone_name) && fixate_in_place) {
				_new_key_origin = Vector3(0.0, 0.0, 0.0);
			}

			_keyframe_value_dict["location"] = _new_key_origin;
			_keyframe_value_dict["rotation"] = _new_key_quat;
			_keyframe_value_dict["scale"] = _new_key_scale;

			new_retargeted_animation->track_set_key_value(_track_inx, _track_key_inx, _keyframe_value_dict);
		}
	}	
	return new_retargeted_animation;
}

void AnimationRetargeting::set_replace_retarget_animations(const bool &p_enabled) {
	replace_retarget_animations = p_enabled;
}

bool AnimationRetargeting::get_replace_retarget_animations() {
	return replace_retarget_animations;
}

void AnimationRetargeting::set_export_animationplayer(const bool &p_enabled) {
	export_animationplayer = p_enabled;
}

bool AnimationRetargeting::get_export_animationplayer() {
	return export_animationplayer;
}

void AnimationRetargeting::set_retarget_mode(AnimationRetargetMode p_retarget_mode) {
	retarget_mode = p_retarget_mode;
}

AnimationRetargeting::AnimationRetargetMode AnimationRetargeting::get_retarget_mode() const {
	return retarget_mode;
}

void AnimationRetargeting::set_export_animations(const bool &p_enabled) {
	export_animations = p_enabled;
}

bool AnimationRetargeting::get_export_animations() {
	return export_animations;
}

void AnimationRetargeting::set_source_skeleton_path(const NodePath &p_source_skeleton_node_path) {
	if (source_skeleton_node_path != p_source_skeleton_node_path) {
		retarget_mapping.clear();
		_retarget_skeleton = nullptr;
		_calculate_retargeting_data = true;
		correction_bone_mapping.clear();
	}
	source_skeleton_node_path = p_source_skeleton_node_path;
}

NodePath AnimationRetargeting::get_source_skeleton_path() {
	return source_skeleton_node_path;
}

void AnimationRetargeting::set_retarget_skeleton_path(const NodePath &p_retarget_skeleton_node_path) {
	if (retarget_skeleton_node_path != p_retarget_skeleton_node_path) {
		retarget_mapping.clear();
		_retarget_skeleton = nullptr;
		_calculate_retargeting_data = true;
		correction_bone_mapping.clear();
	}
	retarget_skeleton_node_path = p_retarget_skeleton_node_path;
}

NodePath AnimationRetargeting::get_retarget_skeleton_path() {
	return retarget_skeleton_node_path;
}

void AnimationRetargeting::set_source_animationplayer_path(const NodePath &p_node) {
	_source_animationplayer = nullptr;
	source_animationplayer_node_path = p_node;
}

NodePath AnimationRetargeting::get_source_animationplayer_path() {
	return source_animationplayer_node_path;
}

void AnimationRetargeting::set_retarget_animationplayer_path(const NodePath &p_node) {
	_retarget_animationplayer = nullptr;
	retarget_animationplayer_node_path = p_node;
}

NodePath AnimationRetargeting::get_retarget_animationplayer_path() {
	return retarget_animationplayer_node_path;
}

void AnimationRetargeting::set_animation_export_format(AnimationExportFormat p_animation_export_format) {
	animation_export_format = p_animation_export_format;
}

AnimationRetargeting::AnimationExportFormat AnimationRetargeting::get_animation_export_format() const {
	return animation_export_format;
}

void AnimationRetargeting::set_export_directory(const String &p_directory) {	
	animation_export_directory = p_directory.strip_edges();
}

String AnimationRetargeting::get_export_directory() const {
	return animation_export_directory;
}

void AnimationRetargeting::set_animation_rename_prefix(const String &p_prefix) {
	animation_rename_prefix = p_prefix.strip_edges();
}

String AnimationRetargeting::get_animation_rename_prefix() const {
	return animation_rename_prefix;
}

void AnimationRetargeting::set_animation_rename_suffix(const String &p_suffix) {
	animation_rename_suffix = p_suffix.strip_edges();
}

String AnimationRetargeting::get_animation_rename_suffix() const {
	return animation_rename_suffix;
}

void AnimationRetargeting::set_retarget_position(const bool &p_enabled) {
	retarget_position = p_enabled;
}

bool AnimationRetargeting::get_retarget_position() {
	return retarget_position;
}

void AnimationRetargeting::set_retarget_rotation(const bool &p_enabled) {
	retarget_rotation = p_enabled;
}

bool AnimationRetargeting::get_retarget_rotation() {
	return retarget_rotation;
}

void AnimationRetargeting::set_retarget_scale(const bool &p_enabled) {
	retarget_scale = p_enabled;
}

bool AnimationRetargeting::get_retarget_scale() {
	return retarget_scale;
}

void AnimationRetargeting::set_root_motion(const bool &p_enabled) {
	root_motion = p_enabled;
}

bool AnimationRetargeting::get_root_motion() {
	return root_motion;
}

void AnimationRetargeting::set_fixate_in_place(const bool &p_enabled) {
	fixate_in_place = p_enabled;
}

bool AnimationRetargeting::get_fixate_in_place() {
	return fixate_in_place;
}

void AnimationRetargeting::set_sync_playback(const bool &p_enabled) {
	sync_playback = p_enabled;
	if (sync_playback && (_source_animationplayer != nullptr) && (_retarget_animationplayer != nullptr)) {
		String current_animation_playback_id = _source_animationplayer->get_assigned_animation();
		if (current_animation_playback_id != "") {
			if (_retarget_animationplayer->has_animation(current_animation_playback_id)) {
				_source_animationplayer->stop();
				_retarget_animationplayer->stop();
				_source_animationplayer->play(current_animation_playback_id);
				_retarget_animationplayer->play(current_animation_playback_id);
			}
		}
	}
}

bool AnimationRetargeting::get_sync_playback() {
	return sync_playback;
}

void AnimationRetargeting::set_source_skeleton_scale(const float &p_source_skeleton_scale) {
	source_skeleton_scale = p_source_skeleton_scale;
	if (source_skeleton_scale <= 0.0) {
		source_skeleton_scale = 1.0;
	}
	_calculate_retargeting_data = true;
}

float AnimationRetargeting::get_source_skeleton_scale() {
	return source_skeleton_scale;
}

void AnimationRetargeting::set_retarget_skeleton_scale(const float &p_retarget_skeleton_scale) {
	retarget_skeleton_scale = p_retarget_skeleton_scale;
	if (retarget_skeleton_scale <= 0.0) {
		retarget_skeleton_scale = 1.0;
	}
	_calculate_retargeting_data = true;
}

float AnimationRetargeting::get_retarget_skeleton_scale() {
	return retarget_skeleton_scale;
}

void AnimationRetargeting::set_ignore_bones(const Array &p_ignore_bones) {
	ignore_bones = p_ignore_bones;
}

Array AnimationRetargeting::get_ignore_bones() {
	return ignore_bones;
}

void AnimationRetargeting::set_source_rig_type(SourceRigType p_source_rig_type) {
	source_rig_type = p_source_rig_type;
	_calculate_retargeting_data = true;
}

AnimationRetargeting::SourceRigType AnimationRetargeting::get_source_rig_type() const {
	return source_rig_type;
}

void AnimationRetargeting::set_target_rig_type(TargetRigType p_target_rig_type) {
	target_rig_type = p_target_rig_type;
	_calculate_retargeting_data = true;
}

AnimationRetargeting::TargetRigType AnimationRetargeting::get_target_rig_type() const {
	return target_rig_type;
}

void AnimationRetargeting::set_custom_bone_mapping(const Dictionary &p_custom_bone_mapping) {
	if (custom_bone_mapping != p_custom_bone_mapping) {
		_calculate_retargeting_data = true;
	}
	custom_bone_mapping = p_custom_bone_mapping;
}

Dictionary AnimationRetargeting::get_custom_bone_mapping() {
	return custom_bone_mapping;
}

void AnimationRetargeting::set_correction_bone(const String &p_correction_bone) {
	correction_bone = p_correction_bone.strip_edges();
	if (correction_bone_mapping.has(correction_bone)) {
		position_correction = correction_bone_mapping[correction_bone].position_correction;
		rotation_correction = correction_bone_mapping[correction_bone].rotation_correction;
		scale_correction = correction_bone_mapping[correction_bone].scale_correction;
	} else {
		position_correction = Vector3(0.0, 0.0, 0.0);
		rotation_correction = Vector3(0.0, 0.0, 0.0);
		scale_correction = Vector3(0.0, 0.0, 0.0);
		CorrectionBoneData correction_bone_save;
		correction_bone_mapping[correction_bone] = correction_bone_save;
	}
}

String AnimationRetargeting::get_correction_bone() const {
	return correction_bone;
}

void AnimationRetargeting::set_position_correction(const Vector3 &p_position_correction) {
	position_correction = p_position_correction;

	if (correction_bone_mapping.has(correction_bone)) {
		correction_bone_mapping[correction_bone].position_correction = position_correction;
	}

	if (correction_mode == AnimationRetargeting::CORRECTION_MODE_ENABLED) {

		String animation_id = _retarget_animationplayer->get_assigned_animation();
		if (animation_id != "") {
			float current_animation_position = _retarget_animationplayer->get_current_animation_position();

			Ref<Animation> source_animation = _source_animationplayer->get_animation(animation_id);

			Ref<Animation> retargeted_animation = _retarget_animation_track(source_animation);

			if (replace_retarget_animations) {
				if (_retarget_animationplayer->has_animation(animation_id)) {
					_retarget_animationplayer->remove_animation(animation_id);
				}
				_retarget_animationplayer->add_animation(animation_id, retargeted_animation);
				_retarget_animationplayer->play(animation_id);
				_retarget_animationplayer->seek(current_animation_position, true);
			}
		}
	}
}

Vector3 AnimationRetargeting::get_position_correction() const {
	return position_correction;
}

void AnimationRetargeting::set_rotation_correction(const Vector3 &p_rotation_correction) {
	rotation_correction = p_rotation_correction;

	if (correction_bone_mapping.has(correction_bone)) {
		correction_bone_mapping[correction_bone].rotation_correction = rotation_correction;
	}

	if (correction_mode == AnimationRetargeting::CORRECTION_MODE_ENABLED) {

		String animation_id = _retarget_animationplayer->get_assigned_animation();
		if (animation_id != "") {
			float current_animation_position = _retarget_animationplayer->get_current_animation_position();

			Ref<Animation> source_animation = _source_animationplayer->get_animation(animation_id);

			Ref<Animation> retargeted_animation = _retarget_animation_track(source_animation);

			if (replace_retarget_animations) {
				if (_retarget_animationplayer->has_animation(animation_id)) {
					_retarget_animationplayer->remove_animation(animation_id);
				}
				_retarget_animationplayer->add_animation(animation_id, retargeted_animation);
				_retarget_animationplayer->play(animation_id);
				_retarget_animationplayer->seek(current_animation_position, true);
			}
		}
	}
}

Vector3 AnimationRetargeting::get_rotation_correction() const {
	return rotation_correction;
}

void AnimationRetargeting::set_scale_correction(const Vector3 &p_scale_correction) {
	scale_correction = p_scale_correction;

	if (correction_bone_mapping.has(correction_bone)) {
		correction_bone_mapping[correction_bone].scale_correction = scale_correction;
	}

	if (correction_mode == AnimationRetargeting::CORRECTION_MODE_ENABLED) {

		String animation_id = _retarget_animationplayer->get_assigned_animation();
		if (animation_id != "") {
			float current_animation_position = _retarget_animationplayer->get_current_animation_position();

			Ref<Animation> source_animation = _source_animationplayer->get_animation(animation_id);

			Ref<Animation> retargeted_animation = _retarget_animation_track(source_animation);

			if (replace_retarget_animations) {
				if (_retarget_animationplayer->has_animation(animation_id)) {
					_retarget_animationplayer->remove_animation(animation_id);
				}
				_retarget_animationplayer->add_animation(animation_id, retargeted_animation);
				_retarget_animationplayer->play(animation_id);
				_retarget_animationplayer->seek(current_animation_position, true);
			}
		}
	}
}

Vector3 AnimationRetargeting::get_scale_correction() const {
	return scale_correction;
}

void AnimationRetargeting::_validate_property(PropertyInfo &property) const {
	if (property.name == "correction_bone") {
		if (_retarget_skeleton) {
			String names("--,");
			for (int i = 0; i < _retarget_skeleton->get_bone_count(); i++) {
				if (i > 0)
					names += ",";
				names += _retarget_skeleton->get_bone_name(i);
			}

			property.hint = PROPERTY_HINT_ENUM;
			property.hint_string = names;
		} else {
			property.hint = PROPERTY_HINT_NONE;
			property.hint_string = "";
		}
	}
}

void AnimationRetargeting::enable_correction_mode() {
	correction_mode = AnimationRetargeting::CORRECTION_MODE_ENABLED;
}
void AnimationRetargeting::disable_correction_mode() {
	correction_mode = AnimationRetargeting::CORRECTION_MODE_DISABLED;
}

void AnimationRetargeting::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_retargeting"), &AnimationRetargeting::start_retargeting);

	ClassDB::bind_method(D_METHOD("set_retarget_mode"), &AnimationRetargeting::set_retarget_mode);
	ClassDB::bind_method(D_METHOD("get_retarget_mode"), &AnimationRetargeting::get_retarget_mode);

	ClassDB::bind_method(D_METHOD("set_export_animations"), &AnimationRetargeting::set_export_animations);
	ClassDB::bind_method(D_METHOD("get_export_animations"), &AnimationRetargeting::get_export_animations);

	ClassDB::bind_method(D_METHOD("set_export_animationplayer"), &AnimationRetargeting::set_export_animationplayer);
	ClassDB::bind_method(D_METHOD("get_export_animationplayer"), &AnimationRetargeting::get_export_animationplayer);

	ClassDB::bind_method(D_METHOD("set_source_skeleton_path", "source_skeleton_node_path"), &AnimationRetargeting::set_source_skeleton_path);
	ClassDB::bind_method(D_METHOD("get_source_skeleton_path"), &AnimationRetargeting::get_source_skeleton_path);

	ClassDB::bind_method(D_METHOD("set_retarget_skeleton_path", "retarget_skeleton_node_path"), &AnimationRetargeting::set_retarget_skeleton_path);
	ClassDB::bind_method(D_METHOD("get_retarget_skeleton_path"), &AnimationRetargeting::get_retarget_skeleton_path);

	ClassDB::bind_method(D_METHOD("set_source_animationplayer_path", "node"), &AnimationRetargeting::set_source_animationplayer_path);
	ClassDB::bind_method(D_METHOD("get_source_animationplayer_path"), &AnimationRetargeting::get_source_animationplayer_path);

	ClassDB::bind_method(D_METHOD("set_retarget_animationplayer_path", "node"), &AnimationRetargeting::set_retarget_animationplayer_path);
	ClassDB::bind_method(D_METHOD("get_retarget_animationplayer_path"), &AnimationRetargeting::get_retarget_animationplayer_path);

	ClassDB::bind_method(D_METHOD("set_animation_rename_prefix", "prefix"), &AnimationRetargeting::set_animation_rename_prefix);
	ClassDB::bind_method(D_METHOD("get_animation_rename_prefix"), &AnimationRetargeting::get_animation_rename_prefix);
	ClassDB::bind_method(D_METHOD("set_animation_rename_suffix", "suffix"), &AnimationRetargeting::set_animation_rename_suffix);
	ClassDB::bind_method(D_METHOD("get_animation_rename_suffix"), &AnimationRetargeting::get_animation_rename_suffix);

	ClassDB::bind_method(D_METHOD("set_replace_retarget_animations", "bool"), &AnimationRetargeting::set_replace_retarget_animations);
	ClassDB::bind_method(D_METHOD("get_replace_retarget_animations"), &AnimationRetargeting::get_replace_retarget_animations);

	ClassDB::bind_method(D_METHOD("set_animation_export_format", "format"), &AnimationRetargeting::set_animation_export_format);
	ClassDB::bind_method(D_METHOD("get_animation_export_format"), &AnimationRetargeting::get_animation_export_format);

	ClassDB::bind_method(D_METHOD("set_source_rig_type", "source_rig_type"), &AnimationRetargeting::set_source_rig_type);
	ClassDB::bind_method(D_METHOD("get_source_rig_type"), &AnimationRetargeting::get_source_rig_type);
	ClassDB::bind_method(D_METHOD("set_target_rig_type", "target_rig_type"), &AnimationRetargeting::set_target_rig_type);
	ClassDB::bind_method(D_METHOD("get_target_rig_type"), &AnimationRetargeting::get_target_rig_type);

	ClassDB::bind_method(D_METHOD("set_export_directory", "directory"), &AnimationRetargeting::set_export_directory);
	ClassDB::bind_method(D_METHOD("get_export_directory"), &AnimationRetargeting::get_export_directory);

	ClassDB::bind_method(D_METHOD("set_retarget_position", "retarget_position"), &AnimationRetargeting::set_retarget_position);
	ClassDB::bind_method(D_METHOD("get_retarget_position"), &AnimationRetargeting::get_retarget_position);
	ClassDB::bind_method(D_METHOD("set_retarget_rotation", "retarget_rotation"), &AnimationRetargeting::set_retarget_rotation);
	ClassDB::bind_method(D_METHOD("get_retarget_rotation"), &AnimationRetargeting::get_retarget_rotation);
	ClassDB::bind_method(D_METHOD("set_retarget_scale", "retarget_scale"), &AnimationRetargeting::set_retarget_scale);
	ClassDB::bind_method(D_METHOD("get_retarget_scale"), &AnimationRetargeting::get_retarget_scale);

	ClassDB::bind_method(D_METHOD("set_root_motion", "root_motion"), &AnimationRetargeting::set_root_motion);
	ClassDB::bind_method(D_METHOD("get_root_motion"), &AnimationRetargeting::get_root_motion);
	ClassDB::bind_method(D_METHOD("set_fixate_in_place", "fixate_in_place"), &AnimationRetargeting::set_fixate_in_place);
	ClassDB::bind_method(D_METHOD("get_fixate_in_place"), &AnimationRetargeting::get_fixate_in_place);
	ClassDB::bind_method(D_METHOD("set_sync_playback", "sync_playback"), &AnimationRetargeting::set_sync_playback);
	ClassDB::bind_method(D_METHOD("get_sync_playback"), &AnimationRetargeting::get_sync_playback);

	ClassDB::bind_method(D_METHOD("set_source_skeleton_scale", "source_skeleton_scale"), &AnimationRetargeting::set_source_skeleton_scale);
	ClassDB::bind_method(D_METHOD("get_source_skeleton_scale"), &AnimationRetargeting::get_source_skeleton_scale);
	ClassDB::bind_method(D_METHOD("set_retarget_skeleton_scale", "retarget_skeleton_scale"), &AnimationRetargeting::set_retarget_skeleton_scale);
	ClassDB::bind_method(D_METHOD("get_retarget_skeleton_scale"), &AnimationRetargeting::get_retarget_skeleton_scale);

	ClassDB::bind_method(D_METHOD("set_ignore_bones", "ignore_bones"), &AnimationRetargeting::set_ignore_bones);
	ClassDB::bind_method(D_METHOD("get_ignore_bones"), &AnimationRetargeting::get_ignore_bones);


	ClassDB::bind_method(D_METHOD("set_correction_bone", "correction_bone"), &AnimationRetargeting::set_correction_bone);
	ClassDB::bind_method(D_METHOD("get_correction_bone"), &AnimationRetargeting::get_correction_bone);
	ClassDB::bind_method(D_METHOD("set_position_correction", "position_correction"), &AnimationRetargeting::set_position_correction);
	ClassDB::bind_method(D_METHOD("get_position_correction"), &AnimationRetargeting::get_position_correction);
	ClassDB::bind_method(D_METHOD("set_rotation_correction", "rotation_correction"), &AnimationRetargeting::set_rotation_correction);
	ClassDB::bind_method(D_METHOD("get_rotation_correction"), &AnimationRetargeting::get_rotation_correction);
	ClassDB::bind_method(D_METHOD("set_scale_correction", "scale_correction"), &AnimationRetargeting::set_scale_correction);
	ClassDB::bind_method(D_METHOD("get_scale_correction"), &AnimationRetargeting::get_scale_correction);

	ClassDB::bind_method(D_METHOD("set_custom_bone_mapping", "custom_bone_mapping"), &AnimationRetargeting::set_custom_bone_mapping);
	ClassDB::bind_method(D_METHOD("get_custom_bone_mapping"), &AnimationRetargeting::get_custom_bone_mapping);

	ClassDB::bind_method(D_METHOD("enable_correction_mode"), &AnimationRetargeting::enable_correction_mode);
	ClassDB::bind_method(D_METHOD("disable_correction_mode"), &AnimationRetargeting::disable_correction_mode);

	ADD_GROUP("Retargeting Settings", "_retargeting_settings");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "source_skeleton_path"), "set_source_skeleton_path", "get_source_skeleton_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "source_animationplayer_path"), "set_source_animationplayer_path", "get_source_animationplayer_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "retarget_skeleton_path"), "set_retarget_skeleton_path", "get_retarget_skeleton_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "retarget_animationplayer_path"), "set_retarget_animationplayer_path", "get_retarget_animationplayer_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "retarget_mode", PROPERTY_HINT_ENUM, "animationplayer,current animation,placeholder,new_source_animations,existing_target_animations"), "set_retarget_mode", "get_retarget_mode");

	ADD_GROUP("Retargeting Options", "_retargeting_options");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "replace_retarget_animations"), "set_replace_retarget_animations", "get_replace_retarget_animations");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "retarget_bone_position"), "set_retarget_position", "get_retarget_position");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "retarget_bone_rotation"), "set_retarget_rotation", "get_retarget_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "retarget_bone_scale"), "set_retarget_scale", "get_retarget_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "root_motion"), "set_root_motion", "get_root_motion");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fixate_in_place"), "set_fixate_in_place", "get_fixate_in_place");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_playback"), "set_sync_playback", "get_sync_playback");	
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "source_skeleton_scale"), "set_source_skeleton_scale", "get_source_skeleton_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "retarget_skeleton_scale"), "set_retarget_skeleton_scale", "get_retarget_skeleton_scale");

	ADD_GROUP("Exporting Options", "_exporting_options");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "export_animations"), "set_export_animations", "get_export_animations");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "export_animationplayer"), "set_export_animationplayer", "get_export_animationplayer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "animation_export_format", PROPERTY_HINT_ENUM, "tres,anim"), "set_animation_export_format", "get_animation_export_format");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "animation_export_directory", PROPERTY_HINT_DIR), "set_export_directory", "get_export_directory");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "animation_rename_prefix"), "set_animation_rename_prefix", "get_animation_rename_prefix");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "animation_rename_suffix"), "set_animation_rename_suffix", "get_animation_rename_suffix");

	ADD_GROUP("Rig Mapping Options", "_rig_mapping_options");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "source_rig_type", PROPERTY_HINT_ENUM, "custom,placeholder,genesis3and8,placeholder,placeholder"), "set_source_rig_type", "get_source_rig_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "target_rig_type", PROPERTY_HINT_ENUM, "custom,placeholder,genesis3and8,placeholder,placeholder"), "set_target_rig_type", "get_target_rig_type");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "custom_bone_mapping"), "set_custom_bone_mapping", "get_custom_bone_mapping");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "ignore_bones"), "set_ignore_bones", "get_ignore_bones");

	ADD_GROUP("Bone Correction Options", "_bone_correction_options");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "correction_bone"), "set_correction_bone", "get_correction_bone");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position_correction"), "set_position_correction", "get_position_correction");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rotation_correction"), "set_rotation_correction", "get_rotation_correction");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "scale_correction"), "set_scale_correction", "get_scale_correction");

	BIND_ENUM_CONSTANT(RETARGET_MODE_ANIMATIONPLAYER);
	BIND_ENUM_CONSTANT(RETARGET_MODE_CURRENT_ANIMATION);
	BIND_ENUM_CONSTANT(RETARGET_MODE_LIVE_MOTION_CAPTURE);
	BIND_ENUM_CONSTANT(RETARGET_MODE_NEW_SOURCE_ANIMATION);
	BIND_ENUM_CONSTANT(RETARGET_MODE_EXISTING_TARGET_ANIMATION);

	BIND_ENUM_CONSTANT(ANIMATION_EXPORT_TRES);
	BIND_ENUM_CONSTANT(ANIMATION_EXPORT_ANIM);

	BIND_ENUM_CONSTANT(SOURCE_RIG_CUSTOM);
	BIND_ENUM_CONSTANT(SOURCE_RIG_RIGIFY2);
	BIND_ENUM_CONSTANT(SOURCE_RIG_GENESIS3AND8);
	BIND_ENUM_CONSTANT(SOURCE_RIG_3DSMAX);
	BIND_ENUM_CONSTANT(SOURCE_RIG_MAKEHUMAN);

	BIND_ENUM_CONSTANT(TARGET_RIG_CUSTOM);
	BIND_ENUM_CONSTANT(TARGET_RIG_RIGIFY2);
	BIND_ENUM_CONSTANT(TARGET_RIG_GENESIS3AND8);
	BIND_ENUM_CONSTANT(TARGET_RIG_3DSMAX);
	BIND_ENUM_CONSTANT(TARGET_RIG_MAKEHUMAN);

	BIND_ENUM_CONSTANT(CORRECTION_MODE_DISABLED);
	BIND_ENUM_CONSTANT(CORRECTION_MODE_ENABLED);

}

AnimationRetargeting::AnimationRetargeting() {

	// GENESIS 3 AND GENESIS 8 SKELETONS

	genesis3and8_bone_mapping["hip"] = "hips";
	genesis3and8_bone_mapping["pelvis"] = "pelvis";

	genesis3and8_bone_mapping["abdomenLower"] = "abdomenLower";
	genesis3and8_bone_mapping["abdomenUpper"] = "abdomenUpper";
	genesis3and8_bone_mapping["chestLower"] = "chestLower";
	genesis3and8_bone_mapping["chestUpper"] = "chestUpper";
	genesis3and8_bone_mapping["neckLower"] = "neckLower";
	genesis3and8_bone_mapping["neckUpper"] = "neckUpper";
	genesis3and8_bone_mapping["head"] = "head";

	genesis3and8_bone_mapping["lCollar"] = "lCollar";
	genesis3and8_bone_mapping["lShldrBend"] = "lShldrBend";
	genesis3and8_bone_mapping["lForearmBend"] = "lForearmBend";
	genesis3and8_bone_mapping["lHand"] = "lHand";

	genesis3and8_bone_mapping["rCollar"] = "rCollar";
	genesis3and8_bone_mapping["rShldrBend"] = "rShldrBend";
	genesis3and8_bone_mapping["rForearmBend"] = "rForearmBend";
	genesis3and8_bone_mapping["rHand"] = "rHand";

	genesis3and8_bone_mapping["lThighBend"] = "lThighBend";
	genesis3and8_bone_mapping["lThighTwist"] = "lThighTwist";
	genesis3and8_bone_mapping["lShin"] = "lShin";
	genesis3and8_bone_mapping["lFoot"] = "lFoot";
	genesis3and8_bone_mapping["lToe"] = "lToe";

	genesis3and8_bone_mapping["rThighBend"] = "rThighBend";
	genesis3and8_bone_mapping["rThighTwist"] = "rThighTwist";
	genesis3and8_bone_mapping["rShin"] = "rShin";
	genesis3and8_bone_mapping["rFoot"] = "rFoot";
	genesis3and8_bone_mapping["rToe"] = "rToe";

	genesis3and8_bone_mapping["lThumb1"] = "lThumb1";
	genesis3and8_bone_mapping["lThumb2"] = "lThumb2";
	genesis3and8_bone_mapping["lThumb3"] = "lThumb3";

	genesis3and8_bone_mapping["lIndex1"] = "lIndex1";
	genesis3and8_bone_mapping["lIndex2"] = "lIndex2";
	genesis3and8_bone_mapping["lIndex3"] = "lIndex3";

	genesis3and8_bone_mapping["lMid1"] = "lMid1";
	genesis3and8_bone_mapping["lMid2"] = "lMid2";
	genesis3and8_bone_mapping["lMid3"] = "lMid3";

	genesis3and8_bone_mapping["lRing1"] = "lRing1";
	genesis3and8_bone_mapping["lRing2"] = "lRing2";
	genesis3and8_bone_mapping["lRing3"] = "lRing3";

	genesis3and8_bone_mapping["lPinky1"] = "lPinky1";
	genesis3and8_bone_mapping["lPinky2"] = "lPinky2";
	genesis3and8_bone_mapping["lPinky3"] = "lPinky3";

	genesis3and8_bone_mapping["rThumb1"] = "rThumb1";
	genesis3and8_bone_mapping["rThumb2"] = "rThumb2";
	genesis3and8_bone_mapping["rThumb3"] = "rThumb3";

	genesis3and8_bone_mapping["rIndex1"] = "rIndex1";
	genesis3and8_bone_mapping["rIndex2"] = "rIndex2";
	genesis3and8_bone_mapping["rIndex3"] = "rIndex3";

	genesis3and8_bone_mapping["rMid1"] = "rMid1";
	genesis3and8_bone_mapping["rMid2"] = "rMid2";
	genesis3and8_bone_mapping["rMid3"] = "rMid3";

	genesis3and8_bone_mapping["rRing1"] = "rRing1";
	genesis3and8_bone_mapping["rRing2"] = "rRing2";
	genesis3and8_bone_mapping["rRing3"] = "rRing3";

	genesis3and8_bone_mapping["rPinky1"] = "rPinky1";
	genesis3and8_bone_mapping["rPinky2"] = "rPinky2";
	genesis3and8_bone_mapping["rPinky3"] = "rPinky3";
}

AnimationRetargeting::~AnimationRetargeting() {
}
