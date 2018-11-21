
add_library(Qt5::genericSensorPlugin MODULE IMPORTED)

_populate_Sensors_plugin_properties(genericSensorPlugin RELEASE "sensors/qtsensors_generic.dll")
_populate_Sensors_plugin_properties(genericSensorPlugin DEBUG "sensors/qtsensors_genericd.dll")

list(APPEND Qt5Sensors_PLUGINS Qt5::genericSensorPlugin)
