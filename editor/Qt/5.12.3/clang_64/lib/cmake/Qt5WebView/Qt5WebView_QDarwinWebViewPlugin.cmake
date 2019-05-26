
add_library(Qt5::QDarwinWebViewPlugin MODULE IMPORTED)

_populate_WebView_plugin_properties(QDarwinWebViewPlugin RELEASE "webview/libqtwebview_darwin.dylib")
_populate_WebView_plugin_properties(QDarwinWebViewPlugin DEBUG "webview/libqtwebview_darwin.dylib")

list(APPEND Qt5WebView_PLUGINS Qt5::QDarwinWebViewPlugin)
