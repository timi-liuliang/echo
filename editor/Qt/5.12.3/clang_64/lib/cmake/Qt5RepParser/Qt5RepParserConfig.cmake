
if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "Qt 5 RepParser module requires at least CMake version 3.1.0")
endif()

get_filename_component(_qt5RepParser_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)


macro(_qt5_RepParser_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::RepParser\" references the file
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


if (NOT TARGET Qt5::RepParser)

    set(_Qt5RepParser_OWN_INCLUDE_DIRS
      "${_qt5RepParser_install_prefix}/lib/QtRepParser.framework"
      "${_qt5RepParser_install_prefix}/lib/QtRepParser.framework/Headers"
    )
    set(Qt5RepParser_PRIVATE_INCLUDE_DIRS "")

    foreach(_dir ${_Qt5RepParser_OWN_INCLUDE_DIRS})
        _qt5_RepParser_check_file_exists(${_dir})
    endforeach()


    set(_Qt5RepParser_MODULE_DEPENDENCIES "Core")


    set(Qt5RepParser_OWN_PRIVATE_INCLUDE_DIRS ${Qt5RepParser_PRIVATE_INCLUDE_DIRS})

    set(_Qt5RepParser_FIND_DEPENDENCIES_REQUIRED)
    if (Qt5RepParser_FIND_REQUIRED)
        set(_Qt5RepParser_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt5RepParser_FIND_DEPENDENCIES_QUIET)
    if (Qt5RepParser_FIND_QUIETLY)
        set(_Qt5RepParser_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt5RepParser_FIND_VERSION_EXACT)
    if (Qt5RepParser_FIND_VERSION_EXACT)
        set(_Qt5RepParser_FIND_VERSION_EXACT EXACT)
    endif()


    foreach(_module_dep ${_Qt5RepParser_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.12.3 ${_Qt5RepParser_FIND_VERSION_EXACT}
                ${_Qt5RepParser_DEPENDENCIES_FIND_QUIET}
                ${_Qt5RepParser_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt5RepParser_FOUND False)
            return()
        endif()

    endforeach()

    set(_Qt5RepParser_LIB_DEPENDENCIES "Qt5::Core")


    add_library(Qt5::RepParser INTERFACE IMPORTED)

    set_property(TARGET Qt5::RepParser PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt5RepParser_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::RepParser PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_REPPARSER_LIB)

    set_property(TARGET Qt5::RepParser PROPERTY INTERFACE_QT_ENABLED_FEATURES )
    set_property(TARGET Qt5::RepParser PROPERTY INTERFACE_QT_DISABLED_FEATURES )

    set(_Qt5RepParser_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt5RepParser_PRIVATE_DIR ${Qt5RepParser_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt5RepParser_PRIVATE_DIR})
            set(_Qt5RepParser_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt5RepParser_PRIVATE_DIRS_EXIST)
        add_library(Qt5::RepParserPrivate INTERFACE IMPORTED)
        set_property(TARGET Qt5::RepParserPrivate PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt5RepParser_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt5RepParser_PRIVATEDEPS)
        foreach(dep ${_Qt5RepParser_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt5RepParser_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::RepParserPrivate PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::RepParser ${_Qt5RepParser_PRIVATEDEPS}
        )
    endif()

    set_target_properties(Qt5::RepParser PROPERTIES
        INTERFACE_LINK_LIBRARIES "${_Qt5RepParser_LIB_DEPENDENCIES}"
    )

    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5RepParser_*Plugin.cmake")

    macro(_populate_RepParser_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt5RepParser_install_prefix}/plugins/${PLUGIN_LOCATION}")
        _qt5_RepParser_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_RepParser_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt5RepParserConfigVersion.cmake")

endif()
