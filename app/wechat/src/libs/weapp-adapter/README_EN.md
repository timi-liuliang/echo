# weapp-adapter
weapp-adapter of Wechat Tiny Game in ES6

----

WeChat game official has provided developers with `weapp-adapter` to allow developers to modify and make it support by themselves for which libraries they use to develop their game on WeChat platform.
(weapp-adapter is at: https://mp.weixin.qq.com/debug/wxagame/dev/tutorial/base/adapter.html )

This project is modified version of `weapp-adapter` in ES6.

----
## Improvement

* Modify `HTMLImageElement` / `HTMLCanvasElement` / `HTMLVideoElement` to make it be able to be detected by `instanceof`
* Export `TouchEvent` globally, and fixed circular dependencies
* Added global pseudo-`MouseEvent` as developer tool needs it
* Added global pseudo-`WebGLRenderingContext`
* Make `XMLHttpRequest` inherits from `EventTarget`
* Added `document.createElementNS`
* Added `EventTarget` property and style to canvas
* Added `clientWidth` and `clientHeight` to canvas
* Support of PixiJS、ThreeJS、Phaser are in progress (WIP)

----

## WeChat game library's known issues

Only listed here for serious issue, and difficult to work around.

* WebGL on Android doesn't support `OES_vertex_array_object ` extension. But instead of returning null/undefined when calling `gl.getExtension("OES_vertex_array_object")`, it returns non-null object. This causes the engine to make mistakes when decide to use `OESVao` or not.
* The size of `gl.createFramebuffer/gl.createTexture` is incorrect on Android with the resolution of canvas.
* WebGL on Android for `stencil` has problem ( `gl.getContextAttributes().stencil !== true` ). This causes PixiJS to not work properly with WebGL mode. Anyway, you can work around in uncleanly way to make it runs but functionality related to Filter, Mask, Graphics won't work. Some of the features that work with ThreeJS also doesn't work.
* When getting information from `WebGLRenderingContext`（antialias、preserveDrawingBuffer、stencil, its returning type should be boolean (true/false), but instead returned as integer(1/0). This will cause problem when using with strict equality comparison `===`.
* Unable to get correct WebGL version. This leads to the use of old version for ThreeJS (Android error of `Cannot read '1' of null). As well as on iOS, but doesn't affect ThreeJS for now.


----
## Instructions

Put all files as found under `src` directory into `js/libs/weapp-adapter` directory.

Whenever you need to import the code, use the following.

```
import './js/libs/weapp-adapter/index.js'
```

----

## Note

* According to ES6 syntax, you can just use `import './js/libs/weapp-adapter/`
(without `index.js`). But for actual test on real device, sometimes it failed.
* This project doesn't include with webpack bundling script. It is recommended to directly reference the source code, then let WeChat game engine (IDE) compile, compress, and convert automatically for you. In fact, the size of code package is smaller compared to if we do it ourselves.