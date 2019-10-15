#!/bin/bash
# build ios app

echo "cmake - generate xcode project"

id -un

xcodebuild -version

# modify environment
# :
export PATH="$PATH:/Applications/CMake.app/Contents/bin:/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin"
echo $PATH

# show cmake version
cmake -version

# cmake - change to output directory
cd ../../solution

# show solution dir
echo "build dir :" $(pwd)

# cmake - generate xcode project
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=RELEASE -DECHO_EDITOR_MODE=FALSE -DECHO_BUILD_PLATFORM_IOS=TRUE -DECHO_BUILD_PLATFORM_MAC=FALSE -G"Xcode" ../
