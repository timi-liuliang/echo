
if (CMAKE_VERSION VERSION_LESS 2.8.3)
    message(FATAL_ERROR "Qt 5 requires at least CMake version 2.8.3")
endif()

get_filename_component(_qt5QuickControls2_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

# For backwards compatibility only. Use Qt5QuickControls2_VERSION instead.
set(Qt5QuickControls2_VERSION_STRING 5.10.0)

set(Qt5QuickControls2_LIBRARIES Qt5::QuickControls2)

macro(_qt5_QuickControls2_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::QuickControls2\" references the file
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

macro(_populate_QuickControls2_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION)
    set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt5QuickControls2_install_prefix}/bin/${LIB_LOCATION}")
    _qt5_QuickControls2_check_file_exists(${imported_location})
    set_target_properties(Qt5::QuickControls2 PROPERTIES
        "INTERFACE_LINK_LIBRARIES" "${_Qt5QuickControls2_LIB_DEPENDENCIES}"
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_Qt5QuickControls2_LIB_DEPENDENCIES}"
    )

    set(imported_implib "${_qt5QuickControls2_install_prefix}/lib/${IMPLIB_LOCATION}")
    _qt5_QuickControls2_check_file_exists(${imported_implib})
    if(NOT "${IMPLIB_LOCATION}" STREQUAL "")
        set_target_properties(Qt5::QuickControls2 PROPERTIES
        "IMPORTED_IMPLIB_${Configuration}" ${imported_implib}
        )
    endif()
endmacro()

if (NOT TARGET Qt5::QuickControls2)

    set(_Qt5QuickControls2_OWN_INCLUDE_DIRS "${_qt5QuickControls2_install_prefix}/include/" "${_qt5QuickControls2_install_prefix}/include/QtQuickControls2")
    set(Qt5QuickControls2_PRIVATE_INCLUDE_DIRS
        "${_qt5QuickControls2_install_prefix}/include/QtQuickControls2/5.10.0"
        "${_qt5QuickControls2_install_prefix}/include/QtQuickControls2/5.10.0/QtQuickControls2"
    )

    foreach(_dir ${_Qt5QuickControls2_OWN_INCLUDE_DIRS})
        _qt5_QuickControls2_check_file_exists(${_dir})
    endforeach()

    # Only check existence of private includes if the Private component is
    # specified.
    list(FIND Qt5QuickControls2_FIND_COMPONENTS Private _check_private)
    if (NOT _check_private STREQUAL -1)
        foreach(_dir ${Qt5QuickControls2_PRIVATE_INCLUDE_DIRS})
            _qt5_QuickControls2_check_file_exists(${_dir})
        endforeach()
    endif()

    set(Qt5QuickControls2_INCLUDE_DIRS ${_Qt5QuickControls2_OWN_INCLUDE_DIRS})

    set(Qt5QuickControls2_DEFINITIONS -DQT_QUICKCONTROLS2_LIB)
    set(Qt5QuickControls2_COMPILE_DEFINITIONS QT_QUICKCONTROLS2_LIB)
    set(_Qt5QuickControls2_MODULE_DEPENDENCIES "Quick;Gui;Core")


    set(Qt5QuickControls2_OWN_PRIVATE_INCLUDE_DIRS ${Qt5QuickControls2_PRIVATE_INCLUDE_DIRS})

    set(_Qt5QuickControls2_FIND_DEPENDENCIES_REQUIRED)
    if (Qt5QuickControls2_FIND_REQUIRED)
        set(_Qt5QuickControls2_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt5QuickControls2_FIND_DEPENDENCIES_QUIET)
    if (Qt5QuickControls2_FIND_QUIETLY)
        set(_Qt5QuickControls2_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt5QuickControls2_FIND_VERSION_EXACT)
    if (Qt5QuickControls2_FIND_VERSION_EXACT)
        set(_Qt5QuickControls2_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt5QuickControls2_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt5QuickControls2_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.10.0 ${_Qt5QuickControls2_FIND_VERSION_EXACT}
                ${_Qt5QuickControls2_DEPENDENCIES_FIND_QUIET}
                ${_Qt5QuickControls2_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt5QuickControls2_FOUND False)
            return()
        endif()

        list(APPEND Qt5QuickControls2_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt5QuickControls2_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5QuickControls2_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt5QuickControls2_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt5QuickControls2_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt5QuickControls2_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5QuickControls2_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5QuickControls2_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5QuickControls2_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5QuickControls2_EXECUTABLE_COMPILE_FLAGS)

    set(_Qt5QuickControls2_LIB_DEPENDENCIES "Qt5::Quick;Qt5::Gui;Qt5::Core")


    add_library(Qt5::QuickControls2 SHARED IMPORTED)

    set_property(TARGET Qt5::QuickControls2 PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt5QuickControls2_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::QuickControls2 PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_QUICKCONTROLS2_LIB)

    set(_Qt5QuickControls2_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt5QuickControls2_PRIVATE_DIR ${Qt5QuickControls2_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt5QuickControls2_PRIVATE_DIR})
            set(_Qt5QuickControls2_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt5QuickControls2_PRIVATE_DIRS_EXIST
        AND NOT CMAKE_VERSION VERSION_LESS 3.0.0 )
        add_library(Qt5::QuickControls2Private INTERFACE IMPORTED)
        set_property(TARGET Qt5::QuickControls2Private PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt5QuickControls2_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt5QuickControls2_PRIVATEDEPS)
        foreach(dep ${_Qt5QuickControls2_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt5QuickControls2_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::QuickControls2Private PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::QuickControls2 ${_Qt5QuickControls2_PRIVATEDEPS}
        )
    endif()

    _populate_QuickControls2_target_properties(RELEASE "Qt5QuickControls2.dll" "Qt5QuickControls2.lib" )



    _populate_QuickControls2_target_properties(DEBUG "Qt5QuickControls2d.dll" "Qt5QuickControls2d.lib" )



    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5QuickControls2_*Plugin.cmake")

    macro(_populate_QuickControls2_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt5QuickControls2_install_prefix}/plugins/${PLUGIN_LOCATION}")
        _qt5_QuickControls2_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_QuickControls2_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt5QuickControls2ConfigVersion.cmake")

endif()
