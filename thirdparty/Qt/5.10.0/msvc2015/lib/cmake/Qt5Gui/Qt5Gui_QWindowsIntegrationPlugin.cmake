
add_library(Qt5::QWindowsIntegrationPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QWindowsIntegrationPlugin RELEASE "platforms/qwindows.dll")
_populate_Gui_plugin_properties(QWindowsIntegrationPlugin DEBUG "platforms/qwindowsd.dll")

list(APPEND Qt5Gui_PLUGINS Qt5::QWindowsIntegrationPlugin)
