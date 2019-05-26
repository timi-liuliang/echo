
add_library(Qt5::PassThruCanBusPlugin MODULE IMPORTED)

_populate_SerialBus_plugin_properties(PassThruCanBusPlugin RELEASE "canbus/libqtpassthrucanbus.dylib")
_populate_SerialBus_plugin_properties(PassThruCanBusPlugin DEBUG "canbus/libqtpassthrucanbus.dylib")

list(APPEND Qt5SerialBus_PLUGINS Qt5::PassThruCanBusPlugin)
