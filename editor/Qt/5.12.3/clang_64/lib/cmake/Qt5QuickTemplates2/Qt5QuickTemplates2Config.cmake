
if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "Qt 5 QuickTemplates2 module requires at least CMake version 3.1.0")
endif()

get_filename_component(_qt5QuickTemplates2_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

# For backwards compatibility only. Use Qt5QuickTemplates2_VERSION instead.
set(Qt5QuickTemplates2_VERSION_STRING 5.12.3)

set(Qt5QuickTemplates2_LIBRARIES Qt5::QuickTemplates2)

macro(_qt5_QuickTemplates2_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::QuickTemplates2\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    endif()
endmacro()

macro(_populate_QuickTemplates2_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION)
    set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt5QuickTemplates2_install_prefix}/lib/${LIB_LOCATION}")
    _qt5_QuickTemplates2_check_file_exists(${imported_location})
    set_target_properties(Qt5::QuickTemplates2 PROPERTIES
        "INTERFACE_LINK_LIBRARIES" "${_Qt5QuickTemplates2_LIB_DEPENDENCIES}"
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_Qt5QuickTemplates2_LIB_DEPENDENCIES}"
    )

endmacro()

if (NOT TARGET Qt5::QuickTemplates2)

    set(_Qt5QuickTemplates2_OWN_INCLUDE_DIRS
      "${_qt5QuickTemplates2_install_prefix}/lib/QtQuickTemplates2.framework"
      "${_qt5QuickTemplates2_install_prefix}/lib/QtQuickTemplates2.framework/Headers"
    )
    set(Qt5QuickTemplates2_PRIVATE_INCLUDE_DIRS
        "${_qt5QuickTemplates2_install_prefix}/lib/QtQuickTemplates2.framework/Versions/5/Headers/5.12.3/"
        "${_qt5QuickTemplates2_install_prefix}/lib/QtQuickTemplates2.framework/Versions/5/Headers/5.12.3/QtQuickTemplates2"
    )

    foreach(_dir ${_Qt5QuickTemplates2_OWN_INCLUDE_DIRS})
        _qt5_QuickTemplates2_check_file_exists(${_dir})
    endforeach()


    set(Qt5QuickTemplates2_INCLUDE_DIRS ${_Qt5QuickTemplates2_OWN_INCLUDE_DIRS})

    set(Qt5QuickTemplates2_DEFINITIONS -DQT_QUICKTEMPLATES2_LIB)
    set(Qt5QuickTemplates2_COMPILE_DEFINITIONS QT_QUICKTEMPLATES2_LIB)
    set(_Qt5QuickTemplates2_MODULE_DEPENDENCIES "Quick;Gui;Core")


    set(Qt5QuickTemplates2_OWN_PRIVATE_INCLUDE_DIRS ${Qt5QuickTemplates2_PRIVATE_INCLUDE_DIRS})

    set(_Qt5QuickTemplates2_FIND_DEPENDENCIES_REQUIRED)
    if (Qt5QuickTemplates2_FIND_REQUIRED)
        set(_Qt5QuickTemplates2_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt5QuickTemplates2_FIND_DEPENDENCIES_QUIET)
    if (Qt5QuickTemplates2_FIND_QUIETLY)
        set(_Qt5QuickTemplates2_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt5QuickTemplates2_FIND_VERSION_EXACT)
    if (Qt5QuickTemplates2_FIND_VERSION_EXACT)
        set(_Qt5QuickTemplates2_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt5QuickTemplates2_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt5QuickTemplates2_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.12.3 ${_Qt5QuickTemplates2_FIND_VERSION_EXACT}
                ${_Qt5QuickTemplates2_DEPENDENCIES_FIND_QUIET}
                ${_Qt5QuickTemplates2_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt5QuickTemplates2_FOUND False)
            return()
        endif()

        list(APPEND Qt5QuickTemplates2_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt5QuickTemplates2_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5QuickTemplates2_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt5QuickTemplates2_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt5QuickTemplates2_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt5QuickTemplates2_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5QuickTemplates2_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5QuickTemplates2_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5QuickTemplates2_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5QuickTemplates2_EXECUTABLE_COMPILE_FLAGS)

    set(_Qt5QuickTemplates2_LIB_DEPENDENCIES "Qt5::Quick;Qt5::Gui;Qt5::Core")


    add_library(Qt5::QuickTemplates2 SHARED IMPORTED)
    set_property(TARGET Qt5::QuickTemplates2 PROPERTY FRAMEWORK 1)

    set_property(TARGET Qt5::QuickTemplates2 PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt5QuickTemplates2_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::QuickTemplates2 PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_QUICKTEMPLATES2_LIB)

    set_property(TARGET Qt5::QuickTemplates2 PROPERTY INTERFACE_QT_ENABLED_FEATURES )
    set_property(TARGET Qt5::QuickTemplates2 PROPERTY INTERFACE_QT_DISABLED_FEATURES )

    set(_Qt5QuickTemplates2_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt5QuickTemplates2_PRIVATE_DIR ${Qt5QuickTemplates2_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt5QuickTemplates2_PRIVATE_DIR})
            set(_Qt5QuickTemplates2_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt5QuickTemplates2_PRIVATE_DIRS_EXIST)
        add_library(Qt5::QuickTemplates2Private INTERFACE IMPORTED)
        set_property(TARGET Qt5::QuickTemplates2Private PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt5QuickTemplates2_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt5QuickTemplates2_PRIVATEDEPS)
        foreach(dep ${_Qt5QuickTemplates2_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt5QuickTemplates2_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::QuickTemplates2Private PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::QuickTemplates2 ${_Qt5QuickTemplates2_PRIVATEDEPS}
        )
    endif()

    _populate_QuickTemplates2_target_properties(RELEASE "QtQuickTemplates2.framework/QtQuickTemplates2" "" )



    _populate_QuickTemplates2_target_properties(DEBUG "QtQuickTemplates2.framework/QtQuickTemplates2" "" )



    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5QuickTemplates2_*Plugin.cmake")

    macro(_populate_QuickTemplates2_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt5QuickTemplates2_install_prefix}/plugins/${PLUGIN_LOCATION}")
        _qt5_QuickTemplates2_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_QuickTemplates2_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt5QuickTemplates2ConfigVersion.cmake")

endif()
