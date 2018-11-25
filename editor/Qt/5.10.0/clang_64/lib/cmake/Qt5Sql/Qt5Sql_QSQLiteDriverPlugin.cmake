
add_library(Qt5::QSQLiteDriverPlugin MODULE IMPORTED)

_populate_Sql_plugin_properties(QSQLiteDriverPlugin RELEASE "sqldrivers/libqsqlite.dylib")
_populate_Sql_plugin_properties(QSQLiteDriverPlugin DEBUG "sqldrivers/libqsqlite.dylib")

list(APPEND Qt5Sql_PLUGINS Qt5::QSQLiteDriverPlugin)
