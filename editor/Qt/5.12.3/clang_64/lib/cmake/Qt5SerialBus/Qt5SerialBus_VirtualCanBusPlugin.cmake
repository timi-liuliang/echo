
add_library(Qt5::VirtualCanBusPlugin MODULE IMPORTED)

_populate_SerialBus_plugin_properties(VirtualCanBusPlugin RELEASE "canbus/libqtvirtualcanbus.dylib")
_populate_SerialBus_plugin_properties(VirtualCanBusPlugin DEBUG "canbus/libqtvirtualcanbus.dylib")

list(APPEND Qt5SerialBus_PLUGINS Qt5::VirtualCanBusPlugin)
