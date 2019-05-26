
add_library(Qt5::QDarwinGamepadBackendPlugin MODULE IMPORTED)

_populate_Gamepad_plugin_properties(QDarwinGamepadBackendPlugin RELEASE "gamepads/libdarwingamepad.dylib")
_populate_Gamepad_plugin_properties(QDarwinGamepadBackendPlugin DEBUG "gamepads/libdarwingamepad.dylib")

list(APPEND Qt5Gamepad_PLUGINS Qt5::QDarwinGamepadBackendPlugin)
