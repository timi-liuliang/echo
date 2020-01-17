import Canvas from './Canvas'

import CommonComputedStyle from './style/CommonComputedStyle'
import getImageComputedStyle from './style/ImageComputedStyle'
import getCanvasComputedStyle from './style/CanvasComputedStyle'
import Event from './Event'

export navigator from './navigator'
export XMLHttpRequest from './XMLHttpRequest'
export WebSocket from './WebSocket'
export Worker from './Worker'
export Image from './Image'
export ImageBitmap from './ImageBitmap'
export Audio from './Audio'
export FileReader from './FileReader'
export HTMLElement from './HTMLElement'
export HTMLImageElement from './HTMLImageElement'
export HTMLCanvasElement from './HTMLCanvasElement'
export HTMLMediaElement from './HTMLMediaElement'
export HTMLAudioElement from './HTMLAudioElement'
export HTMLVideoElement from './HTMLVideoElement'
export WebGLRenderingContext from './WebGLRenderingContext'
export { TouchEvent, PointerEvent, MouseEvent } from './EventIniter/index.js'
export localStorage from './localStorage'
export location from './location'
export * from './WindowProperties'

// 暴露全局的 canvas
GameGlobal.screencanvas = GameGlobal.screencanvas || new Canvas()
const canvas = GameGlobal.screencanvas;

function getComputedStyle(dom) {
    const tagName = dom.tagName;

    if (tagName === "CANVAS") {
        return getCanvasComputedStyle(dom);
    } else if (tagName === "IMG") {
        return getImageComputedStyle(dom);
    }

    return CommonComputedStyle;
}

function scrollTo(x, y) {
    // x = Math.min(window.innerWidth, Math.max(0, x));
    // y = Math.min(window.innerHeight, Math.max(0, y));
    // We can't scroll the page of WeChatTinyGame, so it'll always be 0.

    // window.scrollX = 0;
    // window.scrollY = 0;
}

function scrollBy(dx, dy) {
    window.scrollTo(window.scrollX + dx, window.scrollY + dy);
}

function alert(msg) {
    console.log(msg);
}

function focus() {}

function blur() {}


function eventHandlerFactory() {
    return (res) => {
        const event = new Event('resize')

        event.target = window;
        event.timeStamp = Date.now();
        event.res = res;
        event.windowWidth = res.windowWidth;
        event.windowHeight = res.windowHeight;
        document.dispatchEvent(event);
    }
}

wx.onWindowResize(eventHandlerFactory())


export {
    canvas,
    alert,
    focus,
    blur,
    setTimeout,
    setInterval,
    clearTimeout,
    clearInterval,
    requestAnimationFrame,
    cancelAnimationFrame,
    getComputedStyle,
    scrollTo,
    scrollBy
}
