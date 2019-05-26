
add_library(Qt5::QSvgPlugin MODULE IMPORTED)

_populate_Svg_plugin_properties(QSvgPlugin RELEASE "imageformats/libqsvg.dylib")
_populate_Svg_plugin_properties(QSvgPlugin DEBUG "imageformats/libqsvg.dylib")

list(APPEND Qt5Svg_PLUGINS Qt5::QSvgPlugin)
