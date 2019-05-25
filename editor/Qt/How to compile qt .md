# How To Static Compile Qt 

## Only Src in need to be installed
 
## Command
#### Mac Platform
./Src/configure -static -release -ltcg -optimize-size -no-pch -no-opengl -no-angle -prefix "./static_output" -skip webengine -nomake tools -nomake tools -nomake tests -nomake examples 

#### Windows Platform
../src/configure -static -release -ltcg -optimize-size -no-pch -no-opengl -no-angle -prefix "../static_output" -skip webengine -nomake tools -nomake tools -nomake tests -nomake examples 

# Reference
[1]. [Qt Configure Options](https://doc.qt.io/qt-5/configure-options.html)
