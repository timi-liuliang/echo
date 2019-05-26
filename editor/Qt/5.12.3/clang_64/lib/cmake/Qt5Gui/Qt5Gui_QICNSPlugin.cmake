
add_library(Qt5::QICNSPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QICNSPlugin RELEASE "imageformats/libqicns.dylib")
_populate_Gui_plugin_properties(QICNSPlugin DEBUG "imageformats/libqicns.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QICNSPlugin)
