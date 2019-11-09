# build ios app

echo "cmake - generate visual studio project"

# show cmake version
cmake -version

# cmake - change to output directory
mkdir build
cd build

# show solution dir
# echo "build dir :" $(pwd)

# cmake - generate xcode project
cmake -DCMAKE_BUILD_TYPE=RELEASE -DECHO_EDITOR_MODE=FALSE -G"Visual Studio 15 2017 Win64" ../
