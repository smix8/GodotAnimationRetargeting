# Godot Animation Retargeting

![animationretargeting_banner](https://user-images.githubusercontent.com/52464204/113527015-6244d300-95bc-11eb-9246-98c2e5710284.png)

Animation Retargeting module for Godot Game Engine.

The module is work in progress, only use the master/main branch for testing.

See [Godot 4.x](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_4.x) and [Godot 3.x](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_3.x) branch respectively.

Due to popular demand an addon version made with GDScript ~~is~~ will be available (at some point). This version does not have the full feature set due to performance restrictions, addons are just to slow.

See [Godot 4.x GDScript](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_4.x_gdscript) and [Godot 3.x GDScript](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_3.x_gdscript) branch respectively.

Introduces a new Node Type that can be used to transfer Animation data authored for one Skeleton to another Skeleton inside the Godot Editor or at runtime in your game project.

## Features:
- New Godot Node type to retarget animations in both editor as well as runtime in exported games.
- Transfer Animations and entire AnimationsPlayers between two similar Skeletons with a button press.
- Editing interface to correct bone errors for entire animations.
- Bake options to create new skeleton animation variants from existing animations.
- Live update to see all changes and corrections immediately inside the editor.
- Runtime retargeting API to save performance and precious filesize in exported games.
- Support for custom bone mappings and remappings to transfer animations to different skeletons.
- ~~Preset bone mappings for some well established skeleton rigs from DCC tools or vendors.~~ (in development)
- ~~Custom editor dock and interface to enhance your animation workflow in Godot.~~ (in development)
- Ruby colors!

![animationretargeting_demo_anim](https://user-images.githubusercontent.com/52464204/113527008-5eb14c00-95bc-11eb-871b-3aea2ea436ab.gif)

## Installation

1. Download and compile Godot from source and assure it runs on your hardware.
See official Godot documentation for instructions how to [compile Godot from source](https://docs.godotengine.org/en/latest/development/compiling/index.html).
2. Download repository branch for your Godot version ([Godot 4.x](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_4.x)/[Godot 3.x](https://github.com/smix8/GodotAnimationRetargeting/tree/godot_3.x)) and unpack it.
3. Move the unpacked "animation_retargeting" folder inside your Godot source modules folder.
Your path should end up as "godotsourcefolder/modules/animation_retargeting".
4. Compile Godot from source again with the added AnimationRetargeting module.
5. Inside Godot Editor search for "AnimationRetargeting" Node in the "AddNode" dialog.

## Setup | Usage

1. Add an AnimationRetargeting Node in your project.

2. Add NodePath to your source skeleton.

3. Add NodePath to your source skeleton AnimationPlayer.

4. Add NodePath to your target skeleton.

5. Add NodePath to your target skeleton AnimationPlayer.

6. Customize retargeting and export settings.

7. Make manual backups of all your Animation files and AnimationPlayers.

8. Press "Retarget Animation" button in the editor or call start_retargeting() function.

9. To use custom bone mappings load your {"source_bone_name":"target_bone_name"} Dictionary with set_custom_bone_mapping() before you call start_retargeting().
It is better to save you custom bone mapping Dictionary in a script file (or read from JSON) so no progress is lost when the Godot Inspector bugs out.

10. Hunt bugs!

Refer to AnimationRetargeting Node documentation inside Godot Editor for more function and property explanations.

See below more recent devlog videos for current usage.

![animationretargeting_node_inspector](https://user-images.githubusercontent.com/52464204/113527018-640e9680-95bc-11eb-9f48-033a829faa4c.png)

Devlog Video #5 Module backported to Godot 3.3

[![devlog5](https://img.youtube.com/vi/EJCHrdZrhOI/hqdefault.jpg)](https://youtu.be/EJCHrdZrhOI)

Devlog Video #4 Correcting bones and creating new animation variants

[![devlog4](https://img.youtube.com/vi/PRJbesKeBDY/hqdefault.jpg)](https://youtu.be/PRJbesKeBDY)

Devlog Video #3 Retargeting animations to multiple characters

[![devlog3](https://img.youtube.com/vi/7uf0NVnMcb4/hqdefault.jpg)](https://youtu.be/7uf0NVnMcb4)


## License
MIT
