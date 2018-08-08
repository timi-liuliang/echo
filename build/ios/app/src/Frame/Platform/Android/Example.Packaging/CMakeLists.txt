MESSAGE( STATUS "Configuring module: Example.Packaging...")

MACRO(GROUP_FILES src_files root_path)
  SET(cur_dir ${root_path})
  FOREACH(group_files ${${src_files}})
    STRING(REGEX REPLACE ${cur_dir}/\(.*\) \\1 sgbd_fpath ${group_files})
    STRING(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
    STRING(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
    STRING(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
    IF(sgbd_nogroup)
      SOURCE_GROUP("" FILES ${group_files})   
    ELSE()
      INCLUDE_DIRECTORIES(${sgbd_group_name})
      SOURCE_GROUP(${sgbd_group_name} FILES ${group_files})   
    ENDIF(sgbd_nogroup) 
  ENDFOREACH(group_files)
ENDMACRO(GROUP_FILES)

SET(PACKAGING_PATH ${LORD_SRC_PATH}/Examples/Frame/Platform/Android/Example.Packaging)
SET(EXAMPLERES_PATH ${LORD_ROOT_PATH}/bin/media)

FILE(GLOB_RECURSE ANDRES_FILE *.*)
FILE(GLOB_RECURSE EXAMPLERES_FILE ${LORD_ROOT_PATH}/bin/media/*.*)

GROUP_FILES(ANDRES_FILE ${PACKAGING_PATH})
GROUP_FILES(EXAMPLERES_FILE ${EXAMPLERES_PATH})

ADD_EXECUTABLE(Example.Packaging VC_MDD_ANDROID_PACKAGE ${ANDRES_FILE} ${EXAMPLERES_FILE})

SET_TARGET_PROPERTIES(Example.Packaging PROPERTIES FOLDER "Examples")
ADD_DEPENDENCIES(Example.Packaging Example.NativeActivity)
