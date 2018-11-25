
add_library(Qt5::Scene2DPlugin MODULE IMPORTED)

_populate_3DRender_plugin_properties(Scene2DPlugin RELEASE "renderplugins/libscene2d.dylib")
_populate_3DRender_plugin_properties(Scene2DPlugin DEBUG "renderplugins/libscene2d.dylib")

list(APPEND Qt53DRender_PLUGINS Qt5::Scene2DPlugin)
