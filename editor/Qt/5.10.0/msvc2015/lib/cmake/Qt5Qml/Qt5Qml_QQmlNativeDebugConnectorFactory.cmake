
add_library(Qt5::QQmlNativeDebugConnectorFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlNativeDebugConnectorFactory RELEASE "qmltooling/qmldbg_native.dll")
_populate_Qml_plugin_properties(QQmlNativeDebugConnectorFactory DEBUG "qmltooling/qmldbg_natived.dll")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlNativeDebugConnectorFactory)
