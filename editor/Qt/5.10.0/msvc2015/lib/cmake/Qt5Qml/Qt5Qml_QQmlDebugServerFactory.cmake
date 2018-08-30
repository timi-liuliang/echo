
add_library(Qt5::QQmlDebugServerFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlDebugServerFactory RELEASE "qmltooling/qmldbg_server.dll")
_populate_Qml_plugin_properties(QQmlDebugServerFactory DEBUG "qmltooling/qmldbg_serverd.dll")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlDebugServerFactory)
