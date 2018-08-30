
add_library(Qt5::QQmlProfilerServiceFactory MODULE IMPORTED)

_populate_Qml_plugin_properties(QQmlProfilerServiceFactory RELEASE "qmltooling/qmldbg_profiler.dll")
_populate_Qml_plugin_properties(QQmlProfilerServiceFactory DEBUG "qmltooling/qmldbg_profilerd.dll")

list(APPEND Qt5Qml_PLUGINS Qt5::QQmlProfilerServiceFactory)
