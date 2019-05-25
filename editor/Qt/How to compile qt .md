# How To Static Compile Qt 

## Only Src is need to be installed

1.排除Qt模块

使用configure的-skip选项可以排除Qt模块，一般情况下模块名就是源码目录中对应的子目录名。有些子目录会包含多个模块，比如说qtconnectivity目录就包含了Qt NFC模块和Qt Bluetooth模块，排除这两个模块需要将-skip qtconnectivity作为配置参数，如下所示。

2.包含或排除特性
-feature-<feature> 和 -no-feature-<feature>选项用于包含和排除特性。可用的<feature>都被罗列在tbase/src/corelib/global/qfeatures.txt文件中。比如，要禁用accessibility特性，可用使用-no-feature-accessibility选项。

3.查看选项
configure -h
 
## Command
#### Mac Platform
./../Src/configure -static -release -platform macx-clang QMAKE_APPLE_DEVICE_ARCHS=x86_64 -ltcg -optimize-size -no-pch -no-opengl -no-angle -no-feature-accessibility -prefix "./../clang_64_static" -skip webengine -skip qtconnectivity -skip qtdoc -nomake tools -nomake tools -nomake tests -nomake examples 

#### Windows Platform
./src/configure -static -release -ltcg -optimize-size -no-pch -no-opengl -no-angle -prefix "./static_output" -skip webengine -skip qtconnectivity -nomake tools -nomake tools -nomake tests -nomake examples 

# Reference
[1]. [Qt Configure Options](https://doc.qt.io/qt-5/configure-options.html)
