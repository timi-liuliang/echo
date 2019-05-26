
add_library(Qt5::QQmlDebugServerFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlDebugServerFactory RELEASE "qmltooling/libqmldbg_server.dylib")
_populate_Qml_plugin_properties(QQmlDebugServerFactory DEBUG "qmltooling/libqmldbg_server.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlDebugServerFactory)
