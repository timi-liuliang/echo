# google libraries

ADD_SUBDIRECTORY(flatbuffers)

IF(ECHO_EDITOR_MODE)
	ADD_SUBDIRECTORY(googletest)
ENDIF()