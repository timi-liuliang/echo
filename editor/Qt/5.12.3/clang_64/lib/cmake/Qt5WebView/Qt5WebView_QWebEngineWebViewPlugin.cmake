
add_library(Qt5::QWebEngineWebViewPlugin MODULE IMPORTED)

_populate_WebView_plugin_properties(QWebEngineWebViewPlugin RELEASE "webview/libqtwebview_webengine.dylib")
_populate_WebView_plugin_properties(QWebEngineWebViewPlugin DEBUG "webview/libqtwebview_webengine.dylib")

list(APPEND Qt5WebView_PLUGINS Qt5::QWebEngineWebViewPlugin)
