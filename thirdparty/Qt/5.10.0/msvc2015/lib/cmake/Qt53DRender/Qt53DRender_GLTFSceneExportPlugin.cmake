
add_library(Qt5::GLTFSceneExportPlugin MODULE IMPORTED)

_populate_3DRender_plugin_properties(GLTFSceneExportPlugin RELEASE "sceneparsers/gltfsceneexport.dll")
_populate_3DRender_plugin_properties(GLTFSceneExportPlugin DEBUG "sceneparsers/gltfsceneexportd.dll")

list(APPEND Qt53DRender_PLUGINS Qt5::GLTFSceneExportPlugin)
