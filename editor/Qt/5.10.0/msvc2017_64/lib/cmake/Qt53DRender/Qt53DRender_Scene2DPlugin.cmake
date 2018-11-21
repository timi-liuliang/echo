
add_library(Qt5::Scene2DPlugin MODULE IMPORTED)

_populate_3DRender_plugin_properties(Scene2DPlugin RELEASE "renderplugins/scene2d.dll")
_populate_3DRender_plugin_properties(Scene2DPlugin DEBUG "renderplugins/scene2dd.dll")

list(APPEND Qt53DRender_PLUGINS Qt5::Scene2DPlugin)
