/* register_types.cpp */

#include "register_types.h"

#include "core/object/class_db.h"
#include "animation_retargeting.h"

#ifdef TOOLS_ENABLED
#include "animation_retargeting_editor_plugin.h"
#endif

void register_animation_retargeting_types() {
	ClassDB::register_class<AnimationRetargeting>();

	#ifdef TOOLS_ENABLED
	EditorPlugins::add_by_type<AnimationRetargetingEditorPlugin>();
	#endif
}

void unregister_animation_retargeting_types() {
   // 
}
