
add_library(Qt5::DSServicePlugin MODULE IMPORTED)

_populate_Multimedia_plugin_properties(DSServicePlugin RELEASE "mediaservice/dsengine.dll")
_populate_Multimedia_plugin_properties(DSServicePlugin DEBUG "mediaservice/dsengined.dll")

list(APPEND Qt5Multimedia_PLUGINS Qt5::DSServicePlugin)
