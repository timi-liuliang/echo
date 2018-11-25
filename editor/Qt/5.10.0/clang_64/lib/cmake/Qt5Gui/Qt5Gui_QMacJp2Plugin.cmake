
add_library(Qt5::QMacJp2Plugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QMacJp2Plugin RELEASE "imageformats/libqmacjp2.dylib")
_populate_Gui_plugin_properties(QMacJp2Plugin DEBUG "imageformats/libqmacjp2.dylib")

list(APPEND Qt5Gui_PLUGINS Qt5::QMacJp2Plugin)
