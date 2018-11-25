
add_library(Qt5::QCoreWlanEnginePlugin MODULE IMPORTED)

_populate_Network_plugin_properties(QCoreWlanEnginePlugin RELEASE "bearer/libqcorewlanbearer.dylib")
_populate_Network_plugin_properties(QCoreWlanEnginePlugin DEBUG "bearer/libqcorewlanbearer.dylib")

list(APPEND Qt5Network_PLUGINS Qt5::QCoreWlanEnginePlugin)
