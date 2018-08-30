
add_library(Qt5::QDebugMessageServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QDebugMessageServiceFactory RELEASE "qmltooling/qmldbg_messages.dll")
_populate_Qml_plugin_properties(QDebugMessageServiceFactory DEBUG "qmltooling/qmldbg_messagesd.dll")

list(APPEND Qt5Qml_PLUGINS Qt5::QDebugMessageServiceFactory)
