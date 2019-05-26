
add_library(Qt5::QM3uPlaylistPlugin MODULE IMPORTED)

_populate_Multimedia_plugin_properties(QM3uPlaylistPlugin RELEASE "playlistformats/libqtmultimedia_m3u.dylib")
_populate_Multimedia_plugin_properties(QM3uPlaylistPlugin DEBUG "playlistformats/libqtmultimedia_m3u.dylib")

list(APPEND Qt5Multimedia_PLUGINS Qt5::QM3uPlaylistPlugin)
