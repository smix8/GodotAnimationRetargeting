tool
extends EditorPlugin

var editor : EditorInterface
var editor_plugin : EditorInspectorPlugin

var animation_retargeting : Node

var scene2d_calculate_retargeting_data_button : Button
var scene2d_start_retargeting_button : Button 
var scene2d_correction_mode_button : Button

var scene3d_calculate_retargeting_data_button : Button
var scene3d_start_retargeting_button : Button 
var scene3d_correction_mode_button : Button

const AnimationRetargetingClass = preload("res://addons/animation_retargeting/AnimationRetargeting.gd")


func _enter_tree():
	
	add_custom_type("AnimationRetargeting", "Node", AnimationRetargetingClass, preload("res://addons/animation_retargeting/ar_icon.png"))
	
	editor = get_editor_interface()
	
	# 2D Editor Window
	
	scene2d_calculate_retargeting_data_button = Button.new()
	scene2d_calculate_retargeting_data_button.set_button_icon(editor.get_base_control().get_icon("Play", "EditorIcons"))
	scene2d_calculate_retargeting_data_button.set_text(tr("Calculate Retargeting Data"))
	scene2d_calculate_retargeting_data_button.hide();
	scene2d_calculate_retargeting_data_button.connect("pressed", self, "_calculate_retargeting_data")
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_calculate_retargeting_data_button)
	
	scene2d_start_retargeting_button = Button.new()
	scene2d_start_retargeting_button.set_button_icon(editor.get_base_control().get_icon("Play", "EditorIcons"))
	scene2d_start_retargeting_button.set_text(tr("Retarget Animations"))
	scene2d_start_retargeting_button.hide()
	scene2d_start_retargeting_button.connect("pressed", self, "_start_retargeting")
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_start_retargeting_button)
	
	scene2d_correction_mode_button = Button.new()
	scene2d_correction_mode_button.set_button_icon(editor.get_base_control().get_icon("Tools", "EditorIcons"))
	scene2d_correction_mode_button.set_text(tr("Correction Mode"))
	scene2d_correction_mode_button.set_toggle_mode(true)
	scene2d_correction_mode_button.hide()
	scene2d_correction_mode_button.connect("pressed", self, "_toggle_correction_mode")
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_correction_mode_button)
	
	# 3D Editor Window
	
	scene3d_calculate_retargeting_data_button = Button.new()
	scene3d_calculate_retargeting_data_button.set_button_icon(editor.get_base_control().get_icon("Play", "EditorIcons"))
	scene3d_calculate_retargeting_data_button.set_text(tr("Calculate Retargeting Data"))
	scene3d_calculate_retargeting_data_button.hide()
	scene3d_calculate_retargeting_data_button.connect("pressed", self, "_calculate_retargeting_data")
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_calculate_retargeting_data_button)
	
	scene3d_start_retargeting_button = Button.new()
	scene3d_start_retargeting_button.set_button_icon(editor.get_base_control().get_icon("Play", "EditorIcons"))
	scene3d_start_retargeting_button.set_text(tr("Retarget Animations"))
	scene3d_start_retargeting_button.hide()
	scene3d_start_retargeting_button.connect("pressed", self, "_start_retargeting")
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_start_retargeting_button)
	
	scene3d_correction_mode_button = Button.new()
	scene3d_correction_mode_button.set_button_icon(editor.get_base_control().get_icon("Tools", "EditorIcons"))
	scene3d_correction_mode_button.set_text(tr("Correction Mode"))
	scene3d_correction_mode_button.set_toggle_mode(true)
	scene3d_correction_mode_button.hide()
	scene3d_correction_mode_button.connect("pressed", self, "_toggle_correction_mode")
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_correction_mode_button)
	
	animation_retargeting = null



func _exit_tree():
	
	# 2D Editor Window
	
	if scene2d_calculate_retargeting_data_button != null:
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_calculate_retargeting_data_button)
	if scene2d_start_retargeting_button != null:
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_start_retargeting_button)
	if scene2d_correction_mode_button != null:
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, scene2d_correction_mode_button)
	
	# 3D Editor Window
	
	if scene3d_calculate_retargeting_data_button != null:
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_calculate_retargeting_data_button)
	if scene3d_start_retargeting_button != null:
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_start_retargeting_button)
	if scene3d_correction_mode_button != null:
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scene3d_correction_mode_button)
	
	remove_custom_type("AnimationRetargeting")


func can_handle(object) -> bool:
	return object is AnimationRetargetingClass

func handles(object) -> bool:
	return object is AnimationRetargetingClass


func edit(object) -> void:
	if object is AnimationRetargetingClass:
		animation_retargeting = object
	
	if object != animation_retargeting:
		animation_retargeting = object
		if scene2d_calculate_retargeting_data_button:
			scene2d_calculate_retargeting_data_button.pressed = false
		if scene2d_start_retargeting_button:
			scene2d_start_retargeting_button.pressed = false
		if scene2d_correction_mode_button:
			scene2d_correction_mode_button.pressed = false
		
		if scene3d_calculate_retargeting_data_button:
			scene3d_calculate_retargeting_data_button.pressed = false
		if scene3d_start_retargeting_button:
			scene3d_start_retargeting_button.pressed = false
		if scene3d_correction_mode_button:
			scene3d_correction_mode_button.pressed = false
	
	_start_retargeting()


func make_visible(p_visible : bool):
	
	if p_visible:
		if scene2d_calculate_retargeting_data_button:
			scene2d_calculate_retargeting_data_button.visible = true
		if scene2d_start_retargeting_button:
			scene2d_start_retargeting_button.visible = true
		if scene2d_correction_mode_button:
			scene2d_correction_mode_button.visible = true
		
		if scene3d_calculate_retargeting_data_button:
			scene3d_calculate_retargeting_data_button.visible = true
		if scene3d_start_retargeting_button:
			scene3d_start_retargeting_button.visible = true
		if scene3d_correction_mode_button:
			scene3d_correction_mode_button.visible = true
	else:
		if scene2d_calculate_retargeting_data_button:
			scene2d_calculate_retargeting_data_button.visible = false
		if scene2d_start_retargeting_button:
			scene2d_start_retargeting_button.visible = false
		if scene2d_correction_mode_button:
			scene2d_correction_mode_button.visible = false
		
		if scene3d_calculate_retargeting_data_button:
			scene3d_calculate_retargeting_data_button.visible = false
		if scene3d_start_retargeting_button:
			scene3d_start_retargeting_button.visible = false
		if scene3d_correction_mode_button:
			scene3d_correction_mode_button.visible = false


func _start_retargeting():
	if not animation_retargeting:
		return
	
	if scene2d_start_retargeting_button.pressed or scene3d_start_retargeting_button.pressed:
			animation_retargeting.start_retargeting()


func _toggle_correction_mode() -> void:
	if not animation_retargeting:
		return
	
	if scene3d_correction_mode_button:
		if scene3d_correction_mode_button.pressed:
			animation_retargeting.enable_correction_mode()
			scene2d_correction_mode_button.pressed = false
	if scene3d_correction_mode_button:
		if !scene3d_correction_mode_button.pressed:
			animation_retargeting.disable_correction_mode()
			scene3d_correction_mode_button.pressed = false
			scene2d_correction_mode_button.pressed = false
			
			
func _calculate_retargeting_data() -> void:
	if not animation_retargeting:
		return
	
