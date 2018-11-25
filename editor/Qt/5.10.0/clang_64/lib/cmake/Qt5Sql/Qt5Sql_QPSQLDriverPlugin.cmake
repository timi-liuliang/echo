
add_library(Qt5::QPSQLDriverPlugin MODULE IMPORTED)

_populate_Sql_plugin_properties(QPSQLDriverPlugin RELEASE "sqldrivers/libqsqlpsql.dylib")
_populate_Sql_plugin_properties(QPSQLDriverPlugin DEBUG "sqldrivers/libqsqlpsql.dylib")

list(APPEND Qt5Sql_PLUGINS Qt5::QPSQLDriverPlugin)
