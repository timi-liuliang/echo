
add_library(Qt5::IOSSensorPlugin MODULE IMPORTED)

_populate_Sensors_plugin_properties(IOSSensorPlugin RELEASE "sensors/libqtsensors_ios.dylib")
_populate_Sensors_plugin_properties(IOSSensorPlugin DEBUG "sensors/libqtsensors_ios.dylib")

list(APPEND Qt5Sensors_PLUGINS Qt5::IOSSensorPlugin)
