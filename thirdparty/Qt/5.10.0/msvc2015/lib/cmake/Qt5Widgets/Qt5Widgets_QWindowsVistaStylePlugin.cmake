
add_library(Qt5::QWindowsVistaStylePlugin MODULE IMPORTED)

_populate_Widgets_plugin_properties(QWindowsVistaStylePlugin RELEASE "styles/qwindowsvistastyle.dll")
_populate_Widgets_plugin_properties(QWindowsVistaStylePlugin DEBUG "styles/qwindowsvistastyled.dll")

list(APPEND Qt5Widgets_PLUGINS Qt5::QWindowsVistaStylePlugin)
