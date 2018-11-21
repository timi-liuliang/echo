
add_library(Qt5::QNativeWifiEnginePlugin MODULE IMPORTED)

_populate_Network_plugin_properties(QNativeWifiEnginePlugin RELEASE "bearer/qnativewifibearer.dll")
_populate_Network_plugin_properties(QNativeWifiEnginePlugin DEBUG "bearer/qnativewifibearerd.dll")

list(APPEND Qt5Network_PLUGINS Qt5::QNativeWifiEnginePlugin)
