# include directories
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH})

# Add a shared library
ADD_LIBRARY(echojni SHARED EchoJni.cpp)

# Link engine libraries
TARGET_LINK_LIBRARIES(echojni box2d freeimage freetype lua physx pugixml spine zlib)