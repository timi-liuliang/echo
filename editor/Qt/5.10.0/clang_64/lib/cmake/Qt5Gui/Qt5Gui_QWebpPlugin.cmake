
add_library(Qt5::QWebpPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QWebpPlugin RELEASE "imageformats/libqwebp.dylib")
_populate_Gui_plugin_properties(QWebpPlugin DEBUG "imageformats/libqwebp.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QWebpPlugin)
