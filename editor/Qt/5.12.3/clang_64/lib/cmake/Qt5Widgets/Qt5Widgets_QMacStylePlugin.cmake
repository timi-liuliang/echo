
add_library(Qt5::QMacStylePlugin MODULE IMPORTED)

_populate_Widgets_plugin_properties(QMacStylePlugin RELEASE "styles/libqmacstyle.dylib")
_populate_Widgets_plugin_properties(QMacStylePlugin DEBUG "styles/libqmacstyle.dylib")

list(APPEND Qt5Widgets_PLUGINS Qt5::QMacStylePlugin)
