
# Add a shared library
ADD_LIBRARY(echo-native SHARED native-lib.cpp)

# Link engine libraries
TARGET_LINK_LIBRARIES(echo-native box2d freeimage freetype lua physx pugixml spine zlib engine)