
add_library(Qt5::QOffscreenIntegrationPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QOffscreenIntegrationPlugin RELEASE "platforms/qoffscreen.dll")
_populate_Gui_plugin_properties(QOffscreenIntegrationPlugin DEBUG "platforms/qoffscreend.dll")

list(APPEND Qt5Gui_PLUGINS Qt5::QOffscreenIntegrationPlugin)
