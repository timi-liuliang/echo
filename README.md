<p align="center">
    <img width="48" height="48" src="https://raw.githubusercontent.com/timi-liuliang/echo/master/editor/echo/Resource/App.ico">
</p>  

# Echo

## Introduction  

**Echo** is a new game engine, which used more industry-standard of nowadays for game development. The new design concept makes the engine simplicity to use. but more powerful.

[Examples](https://github.com/timi-liuliang/echo-examples)  
[Documentation](https://github.com/timi-liuliang/echo/wiki)  

## Download   
|Description|Files|Previous|
|---|---|---|   
|Editor [Win64]|[echo-setup-2020.02.08.exe](https://media.githubusercontent.com/media/timi-liuliang/echo-download/master/download/echo-setup-2020.02.08.exe)|[old version](https://github.com/timi-liuliang/echo-download/tree/master/download)|
|Editor [Mac OS X 10.7 or later]|echo.dmg|
|Examples|[echo-examples-master.zip](https://codeload.github.com/timi-liuliang/echo-examples/zip/master)|

## Build
### [Editor]   
[How to Compile "Echo Engine" From Source Code](https://github.com/timi-liuliang/echo/wiki/Compile-Echo-From-Source-Code)

### [App]   

## Features

### Easy Concept  
Scene manager is easy, No Entiy, No GameObject, No Component, No Prefab. Only Node and NodeTree.

###  Highly Efficient Workflow   
![](https://media.githubusercontent.com/media/timi-liuliang/echo-download/master/images/intro/echo.png)  

### Multi-Platform Support  
iOS Android Html5 Windows Mac Linux Steam  

### New Industry Standard Support
gLtf2.0, Vulkan, Metal, Pbr, Real time ray tracing.

### 2D And 3D Seamless Transition  
Every node can be 2d or 3d. The core difference is the camera and the unit the node use. So you can just switch a node to 2d or 3d easily.

### Easy To Program  
Mostly, you'll use Lua as your main programming language. and also you can use c++ directly. the design of node tree makes the Lua logic code more easy to write. and the embedded Lua editor and embedded document help you write code just in the echo editor.   
Except for Lua, You can also choose use embedded Scratch as the main development language. Which is a type of visual script inspired by [MIT](https://scratch.mit.edu/).  In the echo, Scratch is based on Lua, when running the app, It'll convert to Lua, Make sure it's good both at code merge and running efficiency.   
If you really like other types of script language, you can tell us, or you can support it by modifying the c++ code directly.

![](https://media.githubusercontent.com/media/timi-liuliang/echo-download/master/images/intro/echo1.png)   

### Data Flow Programming Based ShaderEditor
Programming visualization is a very import concept for non-programmers, with the help of DataFlowProgramming framework [nodeeditor](https://github.com/paceholder/nodeeditor), shader programming becomes easier to understand and not so evil anymore.

![](https://media.githubusercontent.com/media/timi-liuliang/echo-download/master/images/intro/shadereditor.png)   

### Configurable Module   
Most of the engine's Functionality was implemented by configurable modules. that means when you release your app, you can just choose the module you really need. which makes your app have smaller size and more efficiency running speed.

### Animate Everything   
With Timeline, You can animate everything. You can not only animate any Object's(Node, Setting, Res) any property. But also you can call any Object's any function.

### Channel References
You can compute the value of one property based on the value of another property, possibly on a different node. This lets you duplicate values, or make values relative to other values, and have Echo automatically update them whenever they change.

### Open Source  
Echo is licensed under MIT license. You can just do what you want as your wish.
