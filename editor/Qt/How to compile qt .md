# How To Static Compile Qt 

## 安装，只需安装源码即可。
 
## 指令
#### Mac 平台
../src/configure -static -release -ltcg -optimize-size -no-pch -no-opengl -no-angle -prefix "../static_output" -skip webengine -nomake tools -nomake tools -nomake tests -nomake examples 

#### Windows 平台
../src/configure -static -release -ltcg -optimize-size -no-pch -no-opengl -no-angle -prefix "../static_output" -skip webengine -nomake tools -nomake tools -nomake tests -nomake examples 

# Reference
[1]. [Qt Configure Options](https://doc.qt.io/qt-5/configure-options.html)