tool
extends Node


enum {
	CORRECTION_MODE_DISABLED,
	CORRECTION_MODE_ENABLED
}

enum {
	ANIMATION_EXPORT_TRES,
	ANIMATION_EXPORT_ANIM
}

enum {
	SOURCE_RIG_CUSTOM,
	SOURCE_RIG_RIGIFY2,
	SOURCE_RIG_GENESIS3AND8,
	SOURCE_RIG_3DSMAX,
	SOURCE_RIG_MAKEHUMAN
}

enum {
	TARGET_RIG_CUSTOM,
	TARGET_RIG_RIGIFY2,
	TARGET_RIG_GENESIS3AND8,
	TARGET_RIG_3DSMAX,
	TARGET_RIG_MAKEHUMAN
}

enum {
	RETARGET_MODE_ANIMATIONPLAYER,
	RETARGET_MODE_CURRENT_ANIMATION
}


export(NodePath) var source_skeleton_node_path setget set_source_skeleton_path, get_source_skeleton_path
export(NodePath) var source_animationplayer_node_path setget set_source_animationplayer_path, get_source_animationplayer_path
export(NodePath) var retarget_skeleton_node_path setget set_retarget_skeleton_path, get_retarget_skeleton_path
export(NodePath) var retarget_animationplayer_node_path setget set_retarget_animationplayer_path, get_retarget_animationplayer_path

export(int, "animationplayer","current animation","placeholder") var retarget_mode : int = RETARGET_MODE_ANIMATIONPLAYER
export(bool) var replace_retarget_animations : bool = true
export(bool) var retarget_position : bool = false
export(bool) var retarget_rotation : bool = true
export(bool) var retarget_scale  : bool = false
export(bool) var root_motion = false
export(bool) var fixate_in_place = false
export(bool) var sync_playback = false setget set_sync_playback, get_sync_playback
export(float) var source_skeleton_scale = 1.0
export(float) var retarget_skeleton_scale = 1.0

export(bool) var export_animations : bool = true
export(bool) var export_animationplayer : bool = true
export(int, ".tres", ".anim") var animation_export_format = ANIMATION_EXPORT_ANIM

export(String, DIR) var animation_export_directory  = "res://" setget set_export_directory, get_export_directory
export(String) var animation_rename_prefix = "" setget set_animation_rename_prefix, get_animation_rename_prefix
export(String) var animation_rename_suffix = "" setget set_animation_rename_suffix, get_animation_rename_suffix

export(int, "custom", "placeholder", "genesis3and8", "placeholder", "placeholder") var source_rig_type = SOURCE_RIG_CUSTOM
export(int, "custom", "placeholder", "genesis3and8", "placeholder", "placeholder") var target_rig_type = TARGET_RIG_CUSTOM

export(Dictionary) var custom_bone_mapping = {} setget set_custom_bone_mapping, get_custom_bone_mapping
export(Array) var ignore_bones = []

export(String) var correction_bone : String = ""
export(Vector3) var position_correction = Vector3(0.0, 0.0, 0.0)
export(Vector3) var rotation_correction = Vector3(0.0, 0.0, 0.0)
export(Vector3) var scale_correction = Vector3(0.0, 0.0, 0.0)


func has_retargeting_data() -> bool:
	return not retarget_mapping.empty()


func _valid_setup() -> bool:
	_source_skeleton = get_node(source_skeleton_node_path)
	_source_animationplayer = get_node(source_animationplayer_node_path)
	_retarget_skeleton = get_node(retarget_skeleton_node_path)
	_retarget_animationplayer = get_node(retarget_animationplayer_node_path)
	if _source_skeleton == null:
		print_debug("Missing Source Skeleton, did you pick the wrong nodepath?")
	if _source_animationplayer == null:
		print_debug("Missing Source AnimationPlayer, did you pick the wrong nodepath?")
	if _retarget_skeleton == null:
		print_debug("Missing Retarget Skeleton, did you pick the wrong nodepath?")
	if _source_skeleton == null or _source_animationplayer == null or _retarget_skeleton == null:
		retarget_mapping.clear()
		return false
	if retarget_mode == RETARGET_MODE_CURRENT_ANIMATION:
		var current_animation_playback_id : String = _source_animationplayer.get_current_animation()
		if current_animation_playback_id == "":
			print_debug("Failed to get playing Animation from AnimationPlayer. Animation needs to play currently or else AnimationPlayer returns an empty animation_id.")
			return false
	return true


func start_retargeting() -> void:
	if not _valid_setup():
		return
	for bone_inx in _source_skeleton.get_bone_count():
		_source_skeleton.set_bone_custom_pose(bone_inx, Transform())
		_source_skeleton.set_bone_pose(bone_inx, Transform())
	
	for bone_inx in _retarget_skeleton.get_bone_count():
		_retarget_skeleton.set_bone_custom_pose(bone_inx, Transform())
		_retarget_skeleton.set_bone_pose(bone_inx, Transform())
	
	var current_animation_playback_id : String = _source_animationplayer.get_current_animation()
	
	if calculate_retargeting_data():
		retarget_skeleton_animations()
	
	if _source_animationplayer.has_animation(current_animation_playback_id):
		if sync_playback:
			_source_animationplayer.stop()
		_source_animationplayer.play(current_animation_playback_id)
	
	if _retarget_animationplayer.has_animation(current_animation_playback_id):
		if sync_playback:
			_retarget_animationplayer.stop()
		_retarget_animationplayer.play(current_animation_playback_id)


func calculate_retargeting_data() -> bool:
	if not _valid_setup():
		return false
		
	_skeleton_scale_mod = 1.0;
	
	if source_skeleton_scale <= 0.0:
		source_skeleton_scale = 1.0
	if retarget_skeleton_scale <= 0.0:
		retarget_skeleton_scale = 1.0
	
	_skeleton_scale_mod = source_skeleton_scale / retarget_skeleton_scale
	
	for source_skeleton_bone_inx in _source_skeleton.get_bone_count():
	
		var bone_name : String = _source_skeleton.get_bone_name(source_skeleton_bone_inx)
		
		if _source_skeleton.get_bone_parent(source_skeleton_bone_inx) == -1:
			_source_root_bone_inx = source_skeleton_bone_inx
			_source_root_bone_name = bone_name
		
		if (source_rig_type == SOURCE_RIG_CUSTOM ) and (target_rig_type == TARGET_RIG_CUSTOM) and (not custom_bone_mapping.empty()):
			if custom_bone_mapping.has(bone_name):
				bone_name = custom_bone_mapping.get(bone_name, "")
			else:
				custom_bone_mapping[bone_name] = "";
		
		var retarget_skeleton_bone_inx : int = _retarget_skeleton.find_bone(bone_name)
		if retarget_skeleton_bone_inx < 0:
			continue
		
		var _source_bone_rest : Transform = _source_skeleton.get_bone_rest(source_skeleton_bone_inx)
		var _target_bone_rest : Transform = _retarget_skeleton.get_bone_rest(retarget_skeleton_bone_inx)
		
		var _source_bone_rest_pos : Vector3 = _source_bone_rest.origin
		var _target_bone_rest_pos : Vector3 = _target_bone_rest.origin
		
		var _source_bone_rest_rot : Quat = _source_bone_rest.basis.get_rotation_quat()
		var _target_bone_rest_rot : Quat = _target_bone_rest.basis.get_rotation_quat()
		
		var position_offset_vector : Vector3 = (_target_bone_rest_pos * _skeleton_scale_mod) - (_source_bone_rest_pos * _skeleton_scale_mod)
		var rotation_offset_quat : Quat = _target_bone_rest_rot.normalized().inverse() * _source_bone_rest_rot.normalized()
		var scale_offset_vector : Vector3 = _source_bone_rest.basis.get_scale() - _target_bone_rest.basis.get_scale()
		
		var od : Dictionary = {}
		od["origin_offset"] = position_offset_vector
		od["quat_offset"] = rotation_offset_quat
		od["scale_offset"] = scale_offset_vector
		retarget_mapping[bone_name] = od
	
	set_custom_bone_mapping(custom_bone_mapping)
	
	if retarget_mapping.empty():
		return false
	return true


func retarget_skeleton_animations() -> void:
	if not _valid_setup():
		return
	
	var export_format_string : String = ".tres"
	if animation_export_format == ANIMATION_EXPORT_ANIM:
		export_format_string = ".anim"
	
	var new_animationplayer : AnimationPlayer
	if export_animationplayer:
		new_animationplayer = AnimationPlayer.new()
	
	var playing_source_animation_id : String = _source_animationplayer.get_current_animation()
	var playing_retarget_animation_id : String = _retarget_animationplayer.get_current_animation()
	
	var animation_list : Array = _source_animationplayer.get_animation_list()
	
	if animation_list.size() > 0:
		for animation_id in animation_list:
			if not _source_animationplayer.has_animation(animation_id):
				continue
			if retarget_mode == RETARGET_MODE_CURRENT_ANIMATION and _source_animationplayer.get_current_animation() != animation_id:
				continue
			var source_animation : Animation = _source_animationplayer.get_animation(animation_id)
			
			var retargeted_animation : Animation = _retarget_animation_track(source_animation)
			
			var new_retargeted_animation_name : String = animation_rename_prefix + animation_id + animation_rename_suffix
			
			if replace_retarget_animations and _retarget_animationplayer.has_animation(new_retargeted_animation_name):
				_retarget_animationplayer.remove_animation(new_retargeted_animation_name)
			if not _retarget_animationplayer.has_animation(new_retargeted_animation_name):
				_retarget_animationplayer.add_animation(new_retargeted_animation_name, retargeted_animation)
			
			if export_animations:
				var save_animation_path : String = animation_export_directory + "/" + new_retargeted_animation_name + export_format_string
				var err = ResourceSaver.save(save_animation_path, retargeted_animation)
				if err != OK:
					print_debug("failed to save retargeted animation to export folder %s" % save_animation_path)
			if export_animationplayer:
				new_animationplayer.add_animation(new_retargeted_animation_name, retargeted_animation)
		
		if export_animationplayer:
			var new_packed_scene : PackedScene = PackedScene.new()
			new_packed_scene.pack(new_animationplayer)
			var save_animationplayer_path : String = animation_export_directory + "/" + animation_rename_prefix + "AnimationPlayer" + animation_rename_suffix + ".scn"
			var err = ResourceSaver.save(save_animationplayer_path, new_packed_scene)
			if err != OK:
				print_debug("failed to save retargeted animationplayer to export folder %s", save_animationplayer_path)
	
	if _source_animationplayer.has_animation(playing_source_animation_id):
		_source_animationplayer.play(playing_source_animation_id)
	
	if _retarget_animationplayer.has_animation(playing_retarget_animation_id):
		_retarget_animationplayer.play(playing_retarget_animation_id)


func _add_missing_bones_in_animation_track(p_new_retargeted_animation : Animation) -> void:
	if not _valid_setup():
		return
	
	var bone_keys : Array = retarget_mapping.keys()
	
	if target_rig_type == TARGET_RIG_CUSTOM and not custom_bone_mapping.empty():
		bone_keys = custom_bone_mapping.values()
	
	if target_rig_type == TARGET_RIG_GENESIS3AND8:
		bone_keys = genesis3and8_bone_mapping.keys()
	
	var found_bone_track : bool = false
	var  retarget_skeleton_string_path : String = ""
	
	for bone_key_inx in bone_keys.size():
		
		var searching_bone_name : String = bone_keys[bone_key_inx]
		
		found_bone_track = false
		
		for _track_inx in p_new_retargeted_animation.get_track_count():
			if not p_new_retargeted_animation.track_get_type(_track_inx) == Animation.TYPE_TRANSFORM:
				continue
			var subname_count : int = p_new_retargeted_animation.track_get_path(_track_inx).get_subname_count()
			if subname_count < 1:
				continue
			var bone_name : String = p_new_retargeted_animation.track_get_path(_track_inx).get_subname(subname_count - 1)
			if bone_name == searching_bone_name:
				found_bone_track = true
				retarget_skeleton_string_path = p_new_retargeted_animation.track_get_path(_track_inx)
				var bone_resource_name : String = ":" + bone_name
				retarget_skeleton_string_path = retarget_skeleton_string_path.replace(bone_resource_name, "")
				break
		
		if not found_bone_track:
			
			var new_track_inx : int = p_new_retargeted_animation.get_track_count() - 1
			if new_track_inx > 0:
				var bone_stringpath : String = retarget_skeleton_string_path + ":" + searching_bone_name
				
				var new_animation_track_bone_nodepath : NodePath = NodePath(bone_stringpath)
				var new_translation : Vector3 = Vector3(0.0, 0.0, 0.0);
				var new_rotation_quat : Quat = Quat();
				var new_scale : Vector3 = Vector3(1.0, 1.0, 1.0);
				
				p_new_retargeted_animation.add_track(Animation.TrackType.TYPE_TRANSFORM, new_track_inx)
				p_new_retargeted_animation.track_set_path(new_track_inx, new_animation_track_bone_nodepath)
				p_new_retargeted_animation.transform_track_insert_key(new_track_inx, 0.0, new_translation, new_rotation_quat, new_scale)
				p_new_retargeted_animation.transform_track_insert_key(new_track_inx, p_new_retargeted_animation.get_length(), new_translation, new_rotation_quat, new_scale)


func set_custom_bone_mapping(p_custom_bone_mapping : Dictionary) -> void:
	custom_bone_mapping = p_custom_bone_mapping

func get_custom_bone_mapping() -> Dictionary:
	return custom_bone_mapping


func _retarget_animation_track(p_source_animation : Animation) -> Animation:
	
	var new_retargeted_animation : Animation
	
	new_retargeted_animation = p_source_animation.duplicate()
	
	new_retargeted_animation.set_path("")
	
	_add_missing_bones_in_animation_track(new_retargeted_animation)
	
	var apply_position_correction : bool = _retarget_skeleton.find_bone(correction_bone) != -1
	
	for _track_inx in new_retargeted_animation.get_track_count():
		
		if not new_retargeted_animation.track_get_type(_track_inx) == Animation.TYPE_TRANSFORM:
			continue
		
		var subname_count : int = new_retargeted_animation.track_get_path(_track_inx).get_subname_count()
		var bone_name : String = new_retargeted_animation.track_get_path(_track_inx).get_subname(subname_count - 1)
		
		if not retarget_mapping.has(bone_name):
			continue
		if ignore_bones.has(bone_name):
			continue
		
		var b : String = new_retargeted_animation.track_get_path(_track_inx).get_subname(subname_count - 1)
		
		if target_rig_type == TARGET_RIG_CUSTOM:
			pass
		elif target_rig_type == TARGET_RIG_RIGIFY2:
			if not rigify2_bone_mapping.has(b):
				continue
		elif target_rig_type == TARGET_RIG_GENESIS3AND8:
			if not genesis3and8_bone_mapping.has(b):
				continue
		elif target_rig_type == TARGET_RIG_3DSMAX:
			if not max3ds_bone_mapping.has(b):
				continue
		elif target_rig_type == TARGET_RIG_MAKEHUMAN:
			if not makehuman_bone_mapping.has(b):
				continue
		
		for _track_key_inx in new_retargeted_animation.track_get_key_count(_track_inx):
			
			if not keep_transform_bones.has(bone_name):
				
				var _keyframe_value_dict : Dictionary = new_retargeted_animation.track_get_key_value(_track_inx, _track_key_inx)
				
				var _key_origin : Vector3 = _keyframe_value_dict["location"]
				var _key_quat : Quat = _keyframe_value_dict["rotation"]
				var _key_scale : Vector3 = _keyframe_value_dict["scale"]
				
				var _bone_dict : Dictionary = retarget_mapping[bone_name]
				
				var _origin_offset : Vector3 = _bone_dict["origin_offset"]
				var _quat_offset : Quat = _bone_dict["quat_offset"]
				var _scale_offset : Vector3= _bone_dict["scale_offset"]
				
				var _new_key_origin : Vector3 = _key_origin;
				var _new_key_quat : Quat = _key_quat;
				var _new_key_scale : Vector3= _key_scale;
				
				if retarget_scale or (bone_name == _source_root_bone_name and root_motion):
					if apply_position_correction and bone_name == correction_bone:
						_new_key_scale = _key_scale + _scale_offset + scale_correction
					else:
						_new_key_scale = _key_scale + _scale_offset
				
				if retarget_position or (bone_name == _source_root_bone_name and root_motion):
					if apply_position_correction and bone_name == correction_bone:
						_new_key_origin = (_key_origin * _skeleton_scale_mod) + _origin_offset + position_correction
					else:
						_new_key_origin = (_key_origin * _skeleton_scale_mod) + _origin_offset
				
				# BONE ROTATION
				if retarget_rotation:
					if apply_position_correction and bone_name == correction_bone:
						var _rotation_correction_rad : Vector3;
						_rotation_correction_rad.x = deg2rad(rotation_correction.x)
						_rotation_correction_rad.y = deg2rad(rotation_correction.y)
						_rotation_correction_rad.z = deg2rad(rotation_correction.z)
						_new_key_quat = (((_quat_offset * _key_quat).normalized()) * Quat(_rotation_correction_rad).normalized()).normalized()
					else:
						_new_key_quat = (_quat_offset * _key_quat).normalized()
						
				if (bone_name == _source_root_bone_name and fixate_in_place):
					_new_key_origin.x = 0.0
					_new_key_origin.z = 0.0
				
				_keyframe_value_dict["location"] = _new_key_origin
				_keyframe_value_dict["rotation"] = _new_key_quat
				_keyframe_value_dict["scale"] = _new_key_scale
				
				new_retargeted_animation.track_set_key_value(_track_inx, _track_key_inx, _keyframe_value_dict)
	
	return new_retargeted_animation


func set_source_skeleton_path(p_source_skeleton_node_path : NodePath) -> void:
	retarget_mapping.clear()
	_source_skeleton = null
	source_skeleton_node_path = p_source_skeleton_node_path

func get_source_skeleton_path() -> NodePath:
	return source_skeleton_node_path


func set_retarget_skeleton_path(p_retarget_skeleton_node_path : NodePath) -> void:
	retarget_mapping.clear()
	_retarget_skeleton = null
	retarget_skeleton_node_path = p_retarget_skeleton_node_path

func get_retarget_skeleton_path() -> NodePath:
	return retarget_skeleton_node_path


func set_source_animationplayer_path(p_source_animationplayer_path : NodePath) -> void:
	_source_animationplayer = null
	source_animationplayer_node_path = p_source_animationplayer_path

func get_source_animationplayer_path() -> NodePath:
	return source_animationplayer_node_path


func set_retarget_animationplayer_path(p_retarget_animationplayer_path : NodePath) -> void:
	_retarget_animationplayer = null
	retarget_animationplayer_node_path = p_retarget_animationplayer_path

func get_retarget_animationplayer_path() -> NodePath:
	return retarget_animationplayer_node_path


func set_export_directory(p_directory : String):
	animation_export_directory = p_directory.strip_edges()

func get_export_directory() -> String:
	return animation_export_directory


func set_animation_rename_prefix(p_prefix : String):
	animation_rename_prefix = p_prefix.strip_edges()

func get_animation_rename_prefix() -> String:
	return animation_rename_prefix


func set_animation_rename_suffix(p_suffix : String):
	animation_rename_suffix = p_suffix.strip_edges()

func get_animation_rename_suffix() -> String:
	return animation_rename_suffix


func enable_correction_mode() -> void:
	correction_mode = CORRECTION_MODE_ENABLED

func disable_correction_mode() -> void:
	correction_mode = CORRECTION_MODE_DISABLED


func set_sync_playback(p_enabled) -> void:
	sync_playback = p_enabled
	if sync_playback and _source_animationplayer and _retarget_animationplayer:
		var current_animation_playback_id : String = _source_animationplayer.get_current_animation()
		if current_animation_playback_id != "":
			if _retarget_animationplayer.has_animation(current_animation_playback_id):
				_source_animationplayer.stop()
				_retarget_animationplayer.stop()
				_source_animationplayer.play(current_animation_playback_id)
				_retarget_animationplayer.play(current_animation_playback_id)

func get_sync_playback() -> bool:
	return sync_playback


var retarget_mapping : Dictionary = {}

var _source_skeleton : Skeleton
var _source_animationplayer : AnimationPlayer
var _retarget_skeleton : Skeleton
var _retarget_animationplayer : AnimationPlayer
var _source_root_bone_name : String
var _source_root_bone_inx : int
var _skeleton_scale_mod = 1.0
var rescale_position : float = 1.0
var correction_mode = CORRECTION_MODE_DISABLED

var keep_transform_bones : Dictionary = {}
var rigify2_bone_mapping : Dictionary = {}
var max3ds_bone_mapping : Dictionary = {}
var makehuman_bone_mapping : Dictionary = {}


var genesis3and8_bone_mapping = {
	"hip" : "hips",
	"pelvis" : "pelvis",
	
	"abdomenLower" : "spine",
	"abdomenUpper" : "spine-1",
	"chestLower" : "chest",
	"chestUpper" : "chestUpper",
	"neckLower" : "neck",
	"neckUpper" : "neckUpper", 
	"head" : "head", 
	
	"lCollar" : "shoulder.L",
	"lShldrBend" : "upper_arm.L", 
	"lForearmBend" : "forearm.L",
	"lHand" : "hand.L",

	"rCollar" : "shoulder.R",
	"rShldrBend" : "upper_arm.R",
	"rForearmBend" : "forearm.R",
	"rHand" : "hand.R",

	"lThighBend" : "thigh.L",
	"lThighTwist" : "thigh_twist.L",
	"lShin" : "shin.L",
	"lFoot" : "foot.L", 
	"lToe" : "toe.L",

	"rThighBend" : "thigh.R",
	"rShin" : "shin.R",
	"rFoot" : "foot.R", 
	"rToe" : "toe.R",

	"lThumb1" : "f_thumb.01.L",
	"lThumb2" : "f_thumb.02.L",
	"lThumb3" : "f_thumb.03.L",

	"lIndex1" : "f_index.01.L",
	"lIndex2" : "f_index.02.L",
	"lIndex3" : "f_index.03.L",

	"lMid1" : "f_middle.01.L",
	"lMid2" : "f_middle.02.L",
	"lMid3" : "f_middle.03.L",

	"lRing1" : "f_ring.01.L",
	"lRing2" : "f_ring.02.L",
	"lRing3" : "f_ring.03.L",

	"lPinky1" : "f_pinky.01.L",
	"lPinky2" : "f_pinky.02.L",
	"lPinky3" : "f_pinky.03.L",	

	"rThumb1" : "f_thumb.01.R",
	"rThumb2" : "f_thumb.02.R",
	"rThumb3" : "f_thumb.03.R",

	"rIndex1" : "f_index.01.R",
	"rIndex2" : "f_index.02.R",
	"rIndex3" : "f_index.03.R",

	"rMid1" : "f_middle.01.R",
	"rMid2" : "f_middle.02.R",
	"rMid3" : "f_middle.03.R",

	"rRing1" : "f_ring.01.R",
	"rRing2" : "f_ring.02.R",
	"rRing3" : "f_ring.03.R",

	"rPinky1" : "f_pinky.01.R",
	"rPinky2" : "f_pinky.02.R",
	"rPinky3" : "f_pinky.03.R"
	}
