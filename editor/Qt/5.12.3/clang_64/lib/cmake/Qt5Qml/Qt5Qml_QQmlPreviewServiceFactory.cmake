
add_library(Qt5::QQmlPreviewServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlPreviewServiceFactory RELEASE "qmltooling/libqmldbg_preview.dylib")
_populate_Qml_plugin_properties(QQmlPreviewServiceFactory DEBUG "qmltooling/libqmldbg_preview.dylib")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlPreviewServiceFactory)
