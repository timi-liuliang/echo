
add_library(Qt5::QPSQLDriverPlugin MODULE IMPORTED)

_populate_Sql_plugin_properties(QPSQLDriverPlugin RELEASE "sqldrivers/qsqlpsql.dll")
_populate_Sql_plugin_properties(QPSQLDriverPlugin DEBUG "sqldrivers/qsqlpsqld.dll")

list(APPEND Qt5Sql_PLUGINS Qt5::QPSQLDriverPlugin)
