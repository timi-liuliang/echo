CONFIG+= debug shared rtti no_plugin_manifest directwrite directwrite2 qpa
host_build {
    QT_ARCH = i386
    QT_TARGET_ARCH = i386
} else {
    QT_ARCH = i386
}
QT_CONFIG += force_asserts minimal-config small-config medium-config large-config full-config debug_and_release build_all release debug shared zlib dynamicgl png doubleconversion freetype harfbuzz build_all accessibility opengl dbus audio-backend directwrite directwrite2 native-gestures qpa concurrent
#versioning 
QT_VERSION = 5.7.0
QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 7
QT_PATCH_VERSION = 0

QT_EDITION = OpenSource
