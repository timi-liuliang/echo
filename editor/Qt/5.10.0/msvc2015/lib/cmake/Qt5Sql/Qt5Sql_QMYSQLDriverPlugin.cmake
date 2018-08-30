
add_library(Qt5::QMYSQLDriverPlugin MODULE IMPORTED)

_populate_Sql_plugin_properties(QMYSQLDriverPlugin RELEASE "sqldrivers/qsqlmysql.dll")
_populate_Sql_plugin_properties(QMYSQLDriverPlugin DEBUG "sqldrivers/qsqlmysqld.dll")

list(APPEND Qt5Sql_PLUGINS Qt5::QMYSQLDriverPlugin)
