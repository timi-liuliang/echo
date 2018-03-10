QT_BUILD_PARTS += libs tools examples
QT_SKIP_MODULES += qtwebengine
QT_QCONFIG_PATH = 

host_build {
    QT_CPU_FEATURES.i386 =  sse sse2
} else {
    QT_CPU_FEATURES.i386 =  sse sse2
}
QT_COORD_TYPE += double
CONFIG += precompile_header pcre force_debug_info debug sse2 sse3 ssse3 sse4_1 sse4_2 avx avx2 largefile
QMAKE_QT_VERSION_OVERRIDE = 5
sql-plugins    += sqlite
styles         += windows fusion windowsxp windowsvista
