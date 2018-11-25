
add_library(Qt5::QTiffPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QTiffPlugin RELEASE "imageformats/libqtiff.dylib")
_populate_Gui_plugin_properties(QTiffPlugin DEBUG "imageformats/libqtiff.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QTiffPlugin)
