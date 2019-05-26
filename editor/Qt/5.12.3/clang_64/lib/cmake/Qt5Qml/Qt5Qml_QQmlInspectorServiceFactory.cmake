
add_library(Qt5::QQmlInspectorServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlInspectorServiceFactory RELEASE "qmltooling/libqmldbg_inspector.dylib")
_populate_Qml_plugin_properties(QQmlInspectorServiceFactory DEBUG "qmltooling/libqmldbg_inspector.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlInspectorServiceFactory)
