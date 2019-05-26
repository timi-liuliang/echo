
add_library(Qt5::QDebugMessageServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QDebugMessageServiceFactory RELEASE "qmltooling/libqmldbg_messages.dylib")
_populate_Qml_plugin_properties(QDebugMessageServiceFactory DEBUG "qmltooling/libqmldbg_messages.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QDebugMessageServiceFactory)
