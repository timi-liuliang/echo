# google libraries

ADD_SUBDIRECTORY(gemmlowp)
ADD_SUBDIRECTORY(flatbuffers)
ADD_SUBDIRECTORY(tensorflow/lite)

IF(ECHO_EDITOR_MODE)
	ADD_SUBDIRECTORY(googletest)
ENDIF()