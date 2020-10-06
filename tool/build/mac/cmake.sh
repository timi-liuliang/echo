#!/bin/bash
# build ios app

echo "cmake - generate xcode project"

id -un

xcodebuild -version

# modify environment
export PATH="$PATH:/Applications/CMake.app/Contents/bin"
echo $PATH

# show cmake version
cmake -version

# cmake - change to output directory
mkdir xcode
cd xcode

# show solution dir
echo "build dir :" $(pwd)

# cmake - generate xcode project
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=RELEASE -DECHO_EDITOR_MODE=FALSE -DECHO_BUILD_PLATFORM_IOS=FALSE -DECHO_BUILD_PLATFORM_MAC=TRUE -G"Xcode" ../

# link
cd ../
ln -s ./xcode/echo.xcodeproj ./echo.xcodeproj
