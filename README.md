<p align="center">
    <img width="48" height="48" src="https://raw.githubusercontent.com/blab-liuliang/echo/master/editor/echo/Resource/App.ico">
</p>  

# Echo

## Introduction  

**Echo** is a new game engine, which used more industry standard of nowadays for game development. The new design concept makes the engine simplicity to use. but more powerful.

[Examples](https://github.com/blab-liuliang/echo-examples)  
[Documentation](https://github.com/blab-liuliang/echo/wiki)  

## Download   
|Description|Files|Previous|
|---|---|---|   
|Editor [Win32]|[echo-setup-2018.10.09.exe](https://media.githubusercontent.com/media/blab-liuliang/echo-download/master/download/echo-setup-2018.10.09.exe)|[old version](https://github.com/blab-liuliang/echo-download/tree/master/download)|
|Editor [Mac OS X 10.7 or later]|echo.dmg|
|Examples|[echo-examples-master.zip](https://codeload.github.com/blab-liuliang/echo-examples/zip/master)|

## Build
### [Windows Editor]   
1. Install [Visual Studio 2017] [CMake] [Python 3]...
2. EnterFolder "${echo_root_path}/build/editor/windows"  
3. Double click "cmake.bat"
4. Double click "echo.sln"   
### [Windows App]   

## Features

### Easy Concept  
Scene manager is easy, No Entiy, No GameObject, No Component, No Prefab. Only Node and NodeTree. 

###  Highly Efficient Workflow   
![](https://media.githubusercontent.com/media/blab-liuliang/echo-download/master/images/intro/echo.png)  

### Multi-Platform Support  
iOS Android Html5 Windows Mac Linux Steam  

### New Industry Standard Support 
gLtf2.0, Vulkan, Pbr, Real time ray tracing.

### 2D And 3D Seamless Transition  
Every node can be 2d or 3d. The core difference is the camera and the unit the node use. So you can just switch a node to 2d or 3d easily. 

### Easy To Program  
Mostly, you'll use Lua as your main programming language. and also you can use c++ directly. the design of node tree makes the Lua logic code more easy to write. and the embedded Lua editor and embedded document help you write code just in the echo editor.   
Except for Lua, You can also choose use embedded Scratch as the main development language. Which is a type of visual script inspired by [MIT](https://scratch.mit.edu/).  In the echo, Scratch is based on Lua, when running the app, It'll convert to Lua, Make sure it's good both at code merge and running efficiency.   
If you really like other types of script language, you can tell us, or you can support it by modifying the c++ code directly.

![](https://media.githubusercontent.com/media/blab-liuliang/echo-download/master/images/intro/echo1.png)   

### Configurable Module   
Most of the engine's Functionality was implemented by configurable modules. that means when you release your app, you can just choose the module you really need. which makes your app have smaller size and more efficiency running speed.

### Animate Everything   
With Timeline, You can animate everything. You can not only animate any Object's(Node, Setting, Res) any property. But also you can call any Object's any function.

### Open Source  
Echo is licensed under MIT license. You can just do what you want as your wish.
