
if (CMAKE_VERSION VERSION_LESS 2.8.3)
    message(FATAL_ERROR "Qt 5 requires at least CMake version 2.8.3")
endif()

get_filename_component(_qt53DAnimation_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

# For backwards compatibility only. Use Qt53DAnimation_VERSION instead.
set(Qt53DAnimation_VERSION_STRING 5.10.0)

set(Qt53DAnimation_LIBRARIES Qt5::3DAnimation)

macro(_qt5_3DAnimation_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::3DAnimation\" references the file
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

macro(_populate_3DAnimation_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION)
    set_property(TARGET Qt5::3DAnimation APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt53DAnimation_install_prefix}/bin/${LIB_LOCATION}")
    _qt5_3DAnimation_check_file_exists(${imported_location})
    set_target_properties(Qt5::3DAnimation PROPERTIES
        "INTERFACE_LINK_LIBRARIES" "${_Qt53DAnimation_LIB_DEPENDENCIES}"
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_Qt53DAnimation_LIB_DEPENDENCIES}"
    )

    set(imported_implib "${_qt53DAnimation_install_prefix}/lib/${IMPLIB_LOCATION}")
    _qt5_3DAnimation_check_file_exists(${imported_implib})
    if(NOT "${IMPLIB_LOCATION}" STREQUAL "")
        set_target_properties(Qt5::3DAnimation PROPERTIES
        "IMPORTED_IMPLIB_${Configuration}" ${imported_implib}
        )
    endif()
endmacro()

if (NOT TARGET Qt5::3DAnimation)

    set(_Qt53DAnimation_OWN_INCLUDE_DIRS "${_qt53DAnimation_install_prefix}/include/" "${_qt53DAnimation_install_prefix}/include/Qt3DAnimation")
    set(Qt53DAnimation_PRIVATE_INCLUDE_DIRS
        "${_qt53DAnimation_install_prefix}/include/Qt3DAnimation/5.10.0"
        "${_qt53DAnimation_install_prefix}/include/Qt3DAnimation/5.10.0/Qt3DAnimation"
    )

    foreach(_dir ${_Qt53DAnimation_OWN_INCLUDE_DIRS})
        _qt5_3DAnimation_check_file_exists(${_dir})
    endforeach()

    # Only check existence of private includes if the Private component is
    # specified.
    list(FIND Qt53DAnimation_FIND_COMPONENTS Private _check_private)
    if (NOT _check_private STREQUAL -1)
        foreach(_dir ${Qt53DAnimation_PRIVATE_INCLUDE_DIRS})
            _qt5_3DAnimation_check_file_exists(${_dir})
        endforeach()
    endif()

    set(Qt53DAnimation_INCLUDE_DIRS ${_Qt53DAnimation_OWN_INCLUDE_DIRS})

    set(Qt53DAnimation_DEFINITIONS -DQT_3DANIMATION_LIB)
    set(Qt53DAnimation_COMPILE_DEFINITIONS QT_3DANIMATION_LIB)
    set(_Qt53DAnimation_MODULE_DEPENDENCIES "3DRender;3DCore;Gui;Core")


    set(Qt53DAnimation_OWN_PRIVATE_INCLUDE_DIRS ${Qt53DAnimation_PRIVATE_INCLUDE_DIRS})

    set(_Qt53DAnimation_FIND_DEPENDENCIES_REQUIRED)
    if (Qt53DAnimation_FIND_REQUIRED)
        set(_Qt53DAnimation_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt53DAnimation_FIND_DEPENDENCIES_QUIET)
    if (Qt53DAnimation_FIND_QUIETLY)
        set(_Qt53DAnimation_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt53DAnimation_FIND_VERSION_EXACT)
    if (Qt53DAnimation_FIND_VERSION_EXACT)
        set(_Qt53DAnimation_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt53DAnimation_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt53DAnimation_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.10.0 ${_Qt53DAnimation_FIND_VERSION_EXACT}
                ${_Qt53DAnimation_DEPENDENCIES_FIND_QUIET}
                ${_Qt53DAnimation_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt53DAnimation_FOUND False)
            return()
        endif()

        list(APPEND Qt53DAnimation_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt53DAnimation_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt53DAnimation_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt53DAnimation_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt53DAnimation_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt53DAnimation_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt53DAnimation_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt53DAnimation_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt53DAnimation_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt53DAnimation_EXECUTABLE_COMPILE_FLAGS)

    set(_Qt53DAnimation_LIB_DEPENDENCIES "Qt5::3DRender;Qt5::3DCore;Qt5::Gui;Qt5::Core")


    add_library(Qt5::3DAnimation SHARED IMPORTED)

    set_property(TARGET Qt5::3DAnimation PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt53DAnimation_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::3DAnimation PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_3DANIMATION_LIB)

    set(_Qt53DAnimation_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt53DAnimation_PRIVATE_DIR ${Qt53DAnimation_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt53DAnimation_PRIVATE_DIR})
            set(_Qt53DAnimation_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt53DAnimation_PRIVATE_DIRS_EXIST
        AND NOT CMAKE_VERSION VERSION_LESS 3.0.0 )
        add_library(Qt5::3DAnimationPrivate INTERFACE IMPORTED)
        set_property(TARGET Qt5::3DAnimationPrivate PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt53DAnimation_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt53DAnimation_PRIVATEDEPS)
        foreach(dep ${_Qt53DAnimation_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt53DAnimation_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::3DAnimationPrivate PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::3DAnimation ${_Qt53DAnimation_PRIVATEDEPS}
        )
    endif()

    _populate_3DAnimation_target_properties(RELEASE "Qt53DAnimation.dll" "Qt53DAnimation.lib" )



    _populate_3DAnimation_target_properties(DEBUG "Qt53DAnimationd.dll" "Qt53DAnimationd.lib" )



    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt53DAnimation_*Plugin.cmake")

    macro(_populate_3DAnimation_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt53DAnimation_install_prefix}/plugins/${PLUGIN_LOCATION}")
        _qt5_3DAnimation_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_3DAnimation_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt53DAnimationConfigVersion.cmake")

endif()
