
add_library(Qt5::QICOPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QICOPlugin RELEASE "imageformats/qico.dll")
_populate_Gui_plugin_properties(QICOPlugin DEBUG "imageformats/qicod.dll")

list(APPEND Qt5Gui_PLUGINS Qt5::QICOPlugin)
