
add_library(Qt5::QJpegPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QJpegPlugin RELEASE "imageformats/libqjpeg.dylib")
_populate_Gui_plugin_properties(QJpegPlugin DEBUG "imageformats/libqjpeg.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QJpegPlugin)
