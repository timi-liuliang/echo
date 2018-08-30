
add_library(Qt5::QtSensorGesturePlugin MODULE IMPORTED)

_populate_Sensors_plugin_properties(QtSensorGesturePlugin RELEASE "sensorgestures/qtsensorgestures_plugin.dll")
_populate_Sensors_plugin_properties(QtSensorGesturePlugin DEBUG "sensorgestures/qtsensorgestures_plugind.dll")

list(APPEND Qt5Sensors_PLUGINS Qt5::QtSensorGesturePlugin)
