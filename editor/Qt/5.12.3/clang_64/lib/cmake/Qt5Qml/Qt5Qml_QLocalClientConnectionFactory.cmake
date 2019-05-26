
add_library(Qt5::QLocalClientConnectionFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QLocalClientConnectionFactory RELEASE "qmltooling/libqmldbg_local.dylib")
_populate_Qml_plugin_properties(QLocalClientConnectionFactory DEBUG "qmltooling/libqmldbg_local.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QLocalClientConnectionFactory)
