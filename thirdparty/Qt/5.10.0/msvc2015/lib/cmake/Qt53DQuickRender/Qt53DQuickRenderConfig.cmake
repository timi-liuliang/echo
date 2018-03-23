
if (CMAKE_VERSION VERSION_LESS 2.8.3)
    message(FATAL_ERROR "Qt 5 requires at least CMake version 2.8.3")
endif()

get_filename_component(_qt53DQuickRender_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

# For backwards compatibility only. Use Qt53DQuickRender_VERSION instead.
set(Qt53DQuickRender_VERSION_STRING 5.10.0)

set(Qt53DQuickRender_LIBRARIES Qt5::3DQuickRender)

macro(_qt5_3DQuickRender_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::3DQuickRender\" references the file
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

macro(_populate_3DQuickRender_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION)
    set_property(TARGET Qt5::3DQuickRender APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt53DQuickRender_install_prefix}/bin/${LIB_LOCATION}")
    _qt5_3DQuickRender_check_file_exists(${imported_location})
    set_target_properties(Qt5::3DQuickRender PROPERTIES
        "INTERFACE_LINK_LIBRARIES" "${_Qt53DQuickRender_LIB_DEPENDENCIES}"
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_Qt53DQuickRender_LIB_DEPENDENCIES}"
    )

    set(imported_implib "${_qt53DQuickRender_install_prefix}/lib/${IMPLIB_LOCATION}")
    _qt5_3DQuickRender_check_file_exists(${imported_implib})
    if(NOT "${IMPLIB_LOCATION}" STREQUAL "")
        set_target_properties(Qt5::3DQuickRender PROPERTIES
        "IMPORTED_IMPLIB_${Configuration}" ${imported_implib}
        )
    endif()
endmacro()

if (NOT TARGET Qt5::3DQuickRender)

    set(_Qt53DQuickRender_OWN_INCLUDE_DIRS "${_qt53DQuickRender_install_prefix}/include/" "${_qt53DQuickRender_install_prefix}/include/Qt3DQuickRender")
    set(Qt53DQuickRender_PRIVATE_INCLUDE_DIRS
        "${_qt53DQuickRender_install_prefix}/include/Qt3DQuickRender/5.10.0"
        "${_qt53DQuickRender_install_prefix}/include/Qt3DQuickRender/5.10.0/Qt3DQuickRender"
    )

    foreach(_dir ${_Qt53DQuickRender_OWN_INCLUDE_DIRS})
        _qt5_3DQuickRender_check_file_exists(${_dir})
    endforeach()

    # Only check existence of private includes if the Private component is
    # specified.
    list(FIND Qt53DQuickRender_FIND_COMPONENTS Private _check_private)
    if (NOT _check_private STREQUAL -1)
        foreach(_dir ${Qt53DQuickRender_PRIVATE_INCLUDE_DIRS})
            _qt5_3DQuickRender_check_file_exists(${_dir})
        endforeach()
    endif()

    set(Qt53DQuickRender_INCLUDE_DIRS ${_Qt53DQuickRender_OWN_INCLUDE_DIRS})

    set(Qt53DQuickRender_DEFINITIONS -DQT_3DQUICKRENDER_LIB)
    set(Qt53DQuickRender_COMPILE_DEFINITIONS QT_3DQUICKRENDER_LIB)
    set(_Qt53DQuickRender_MODULE_DEPENDENCIES "3DRender;3DQuick;3DCore;Gui;Qml;Core")


    set(Qt53DQuickRender_OWN_PRIVATE_INCLUDE_DIRS ${Qt53DQuickRender_PRIVATE_INCLUDE_DIRS})

    set(_Qt53DQuickRender_FIND_DEPENDENCIES_REQUIRED)
    if (Qt53DQuickRender_FIND_REQUIRED)
        set(_Qt53DQuickRender_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt53DQuickRender_FIND_DEPENDENCIES_QUIET)
    if (Qt53DQuickRender_FIND_QUIETLY)
        set(_Qt53DQuickRender_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt53DQuickRender_FIND_VERSION_EXACT)
    if (Qt53DQuickRender_FIND_VERSION_EXACT)
        set(_Qt53DQuickRender_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt53DQuickRender_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt53DQuickRender_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.10.0 ${_Qt53DQuickRender_FIND_VERSION_EXACT}
                ${_Qt53DQuickRender_DEPENDENCIES_FIND_QUIET}
                ${_Qt53DQuickRender_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt53DQuickRender_FOUND False)
            return()
        endif()

        list(APPEND Qt53DQuickRender_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt53DQuickRender_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt53DQuickRender_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt53DQuickRender_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt53DQuickRender_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt53DQuickRender_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt53DQuickRender_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt53DQuickRender_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt53DQuickRender_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt53DQuickRender_EXECUTABLE_COMPILE_FLAGS)

    set(_Qt53DQuickRender_LIB_DEPENDENCIES "Qt5::3DRender;Qt5::3DQuick;Qt5::3DCore;Qt5::Gui;Qt5::Qml;Qt5::Core")


    add_library(Qt5::3DQuickRender SHARED IMPORTED)

    set_property(TARGET Qt5::3DQuickRender PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt53DQuickRender_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::3DQuickRender PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_3DQUICKRENDER_LIB)

    set(_Qt53DQuickRender_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt53DQuickRender_PRIVATE_DIR ${Qt53DQuickRender_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt53DQuickRender_PRIVATE_DIR})
            set(_Qt53DQuickRender_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt53DQuickRender_PRIVATE_DIRS_EXIST
        AND NOT CMAKE_VERSION VERSION_LESS 3.0.0 )
        add_library(Qt5::3DQuickRenderPrivate INTERFACE IMPORTED)
        set_property(TARGET Qt5::3DQuickRenderPrivate PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt53DQuickRender_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt53DQuickRender_PRIVATEDEPS)
        foreach(dep ${_Qt53DQuickRender_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt53DQuickRender_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::3DQuickRenderPrivate PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::3DQuickRender ${_Qt53DQuickRender_PRIVATEDEPS}
        )
    endif()

    _populate_3DQuickRender_target_properties(RELEASE "Qt53DQuickRender.dll" "Qt53DQuickRender.lib" )



    _populate_3DQuickRender_target_properties(DEBUG "Qt53DQuickRenderd.dll" "Qt53DQuickRenderd.lib" )



    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt53DQuickRender_*Plugin.cmake")

    macro(_populate_3DQuickRender_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt53DQuickRender_install_prefix}/plugins/${PLUGIN_LOCATION}")
        _qt5_3DQuickRender_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_3DQuickRender_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt53DQuickRenderConfigVersion.cmake")

endif()
