
add_library(Qt5::QMacHeifPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QMacHeifPlugin RELEASE "imageformats/libqmacheif.dylib")
_populate_Gui_plugin_properties(QMacHeifPlugin DEBUG "imageformats/libqmacheif.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QMacHeifPlugin)
