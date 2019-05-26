
add_library(Qt5::genericSensorPlugin MODULE IMPORTED)

_populate_Sensors_plugin_properties(genericSensorPlugin RELEASE "sensors/libqtsensors_generic.dylib")
_populate_Sensors_plugin_properties(genericSensorPlugin DEBUG "sensors/libqtsensors_generic.dylib")

list(APPEND Qt5Sensors_PLUGINS Qt5::genericSensorPlugin)
