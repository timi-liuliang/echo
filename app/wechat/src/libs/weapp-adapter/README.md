# weapp-adapter
weapp-adapter of Wechat Tiny Game in ES6

<sub>For english, see [here](https://github.com/finscn/weapp-adapter/blob/master/README_EN.md)<sub>

----

微信小游戏官方提供了一个`weapp-adapter`的示例文件.
(关于什么是 weapp-adapter 详见: https://mp.weixin.qq.com/debug/wxagame/dev/tutorial/base/adapter.html )

本项目则是一个改良的`weapp-adapter`, 基于ES6.
针对 PixiJS、ThreeJS、Babylon、Phaser 等流行框架做适配, 希望可以尽可能完善的支持它们. 当然这将是一个持续的过程.

----

## 改良内容

* 修改 HTMLImageElement / HTMLCanvasElement / HTMLVideoElement 的实现。可通过 instanceof 检测
* 导出全局 TouchEvent, 并解除循环依赖
* 添加全局 伪MoustEvent (开发工具里需要)
* 添加全局 伪WebGLRenderingContext
* XMLHttpRequest 继承 EventTarget, 并支持请求本地文件(相对路径, 使用wx的readFile)
* 添加若干 document 和 window 下的属性与方法
* 为 canvas 添加 EventTarget特性 与 基本的style属性
* 为 canvas 添加 clientWidth/clientHeight/getBoundingClientRect()/focus()/blur() 等常用属性和方法
* 支持 基本的PointerEvent, 并且支持多点触控
* 添加 符合Web习惯的Worker, 但只适用与主线程
* 离屏canvas不支持WebGL模式 (支持的非常糟糕, 相当于不支持)


----

## 微信小游戏引擎 已知问题

（只列出比较严重的、且难以通过hack手段解决的问题）

* (已暂时解决, 但不是最佳方案) 对扩展`EXT_texture_filter_anisotropic`的支持有bug.
执行下列代码:
```
var ext = gl.getExtension("EXT_texture_filter_anisotropic")
    || gl.getExtension("WEBKIT_EXT_texture_filter_anisotropic")
    ||gl.getExtension("MOZ_EXT_texture_filter_anisotropic");
```
 > 此时:  
 > * `ext.TEXTURE_MAX_ANISOTROPY_EXT` 应该为一个数字, 但是小游戏里为 undefined.   
 > * `gl.getParameter(ext.MAX_TEXTURE_MAX_ANISOTROPY_EXT)` 应该为一个数字, 但是小游戏里为 null.   

* (已暂时解决, 但不是最佳方案) 目前小游戏底层在Android下对WebGL的扩展`OES_vertex_array_object `支持有问题，但是执行`gl.getExtension("OES_vertex_array_object")`时返回的却不是`null/undefined`，而是一个非空的对象。导致引擎在使用OES-vao时产生错误。

* Android下`gl.createFramebuffer/gl.createTexture`的大小有误。与canvas的分辨率有关。

* Android下的WebGL对`stencil`的支持有问题( `gl.getContextAttributes().stencil !== true` )。这导致 PixiJS 无法正常使用WebGL模式。虽然通过一些比较丑陋的hack，可以让程序运行，但是某些涉及到 Filter、Mask、Graphics 的功能无法正常使用。在使用ThreeJS的一些高级功能也会出现一些问题。

* 获取 WebGLRenderingContext的信息（antialias、preserveDrawingBuffer、stencil）时，本应该是`布尔类型`，返回的却是数值 1/0, 而不是 true/false 。导致使用严格判断（ === ）时，出现错误。

* 无法正确取得WebGL的版本。导致使用 ThreeJS(老版本)时，Android下直接报错（Cannot read '1' of null）。iOS下取得的版本号有误，但是暂时不影响ThreeJS的使用。

* (iOS only)虽然不支持WebGL 2.0, 但是 canvas.getContext("webgl2") 和 canvas.getContext("experimental-webgl2") 返回的却不是空. 导致某些通过类似代码判断webgl版本的程序出错。

* (已暂时解决, 但不是最佳方案) window.performance.now 返回值的单位不正确。

* wx.onTouchEnd事件的event.touches值不对. 当一个手指抬起后, 这个手指的信息应该从  event.touches 里移除 . 但是实际上并没有移除.

* wx的touch事件的 event.changedTouches和 event.touches列表里 每个元素(Touch对象) 缺少`target`属性

* wx.getStorageSync(key) 的BUG. 当 key不存在时, 该api返回的是 空字符串, 应该返回null.

* 小游戏的`Worker`里不支持`setInterval`和`setTimeout`, 也不支持全局对象, 无法设置跨模块的全局变量。


----

## 使用方法

将`src`下的文件放入小游戏项目中(例如 放入 js/libs/weapp-adapter 目录内)

在需要使用`weapp-adapter`的文件内使用下列代码引入即可.

```
import './js/libs/weapp-adapter/index.js'
```

----

#### 注意:

* 按ES6语法, 理论上可以使用 `import './js/libs/weapp-adapter/`
(不加index.js), 但是实际真机测试发现有些时候不行.
* 本项目没有提供 webpack 编译脚本, 建议直接引用源代码。然后让微信小游戏引擎自己进行编译、压缩、转换。这样其实代码包体积比自行编译还要小一些。

