
add_library(Qt5::AssimpSceneImportPlugin MODULE IMPORTED)

_populate_3DRender_plugin_properties(AssimpSceneImportPlugin RELEASE "sceneparsers/libassimpsceneimport.dylib")
_populate_3DRender_plugin_properties(AssimpSceneImportPlugin DEBUG "sceneparsers/libassimpsceneimport.dylib")

list(APPEND Qt53DRender_PLUGINS Qt5::AssimpSceneImportPlugin)
