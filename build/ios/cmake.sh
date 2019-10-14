#!/bin/bash
# build ios app

echo "cmake - generate xcode project"

# cmake - change to output directory
cd ../../solution

# show solution dir
echo "build dir :" $(pwd)

# cmake - generate xcode project
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=RELEASE -DECHO_EDITOR_MODE=FALSE -DECHO_BUILD_PLATFORM_IOS=TRUE -DECHO_BUILD_PLATFORM_MAC=FALSE -G"Xcode" ../
