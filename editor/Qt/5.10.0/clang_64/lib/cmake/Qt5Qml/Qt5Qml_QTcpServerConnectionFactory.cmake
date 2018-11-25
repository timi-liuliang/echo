
add_library(Qt5::QTcpServerConnectionFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QTcpServerConnectionFactory RELEASE "qmltooling/libqmldbg_tcp.dylib")
_populate_Qml_plugin_properties(QTcpServerConnectionFactory DEBUG "qmltooling/libqmldbg_tcp.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QTcpServerConnectionFactory)
