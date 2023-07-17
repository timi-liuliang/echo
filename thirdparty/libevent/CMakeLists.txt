# Set Module Name
SET(MODULE_NAME libevent)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# Include Directories
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/compat)

# 
IF(${LORD_PLATFORM_WIN32})
	SET(SYS_SRC win32select.c evthread_win32.c buffer_iocp.c event_iocp.c bufferevent_async.c
		strlcpy.c select.c poll.c devpoll.c kqueue.c epoll.c evport.c signal.c
	)
ELSE()

ENDIF()

FILE(GLOB_RECURSE INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

SET(CORE_SRC
	buffer.c
	bufferevent.c
	bufferevent_filter.c
	bufferevent_pair.c
	bufferevent_ratelim.c
	bufferevent_sock.c
	event.c
	evmap.c
	evthread.c
	evutil.c
	evutil_rand.c
	evutil_time.c
	listener.c
	log.c
	signal.c
	${SYS_SRC}
	)
	
SET(EXTRAS_SRC evdns.c event_tagging.c evrpc.c http.c)


ADD_DEFINITIONS(-DSOCK_NONBLOCK)
	
# Add library
ADD_LIBRARY(${MODULE_NAME} STATIC ${CORE_SRC} ${SYS_SRC} ${EXTRAS_SRC} ${INCLUDE_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

MESSAGE(STATUS "configure success!")