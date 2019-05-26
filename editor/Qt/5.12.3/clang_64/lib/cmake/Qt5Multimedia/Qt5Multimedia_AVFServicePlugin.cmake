
add_library(Qt5::AVFServicePlugin MODULE IMPORTED)

_populate_Multimedia_plugin_properties(AVFServicePlugin RELEASE "mediaservice/libqavfcamera.dylib")
_populate_Multimedia_plugin_properties(AVFServicePlugin DEBUG "mediaservice/libqavfcamera.dylib")

list(APPEND Qt5Multimedia_PLUGINS Qt5::AVFServicePlugin)
