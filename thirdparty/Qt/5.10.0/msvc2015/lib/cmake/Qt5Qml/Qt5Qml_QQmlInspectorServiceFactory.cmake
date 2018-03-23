
add_library(Qt5::QQmlInspectorServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlInspectorServiceFactory RELEASE "qmltooling/qmldbg_inspector.dll")
_populate_Qml_plugin_properties(QQmlInspectorServiceFactory DEBUG "qmltooling/qmldbg_inspectord.dll")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlInspectorServiceFactory)
