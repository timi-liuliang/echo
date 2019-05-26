
add_library(Qt5::QODBCDriverPlugin MODULE IMPORTED)

_populate_Sql_plugin_properties(QODBCDriverPlugin RELEASE "sqldrivers/libqsqlodbc.dylib")
_populate_Sql_plugin_properties(QODBCDriverPlugin DEBUG "sqldrivers/libqsqlodbc.dylib")

list(APPEND Qt5Sql_PLUGINS Qt5::QODBCDriverPlugin)
