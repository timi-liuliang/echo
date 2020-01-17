import { noop } from '../util/index.js'
import Event from '../Event'

export default class TouchEvent extends Event {

    touches = []
    targetTouches = []
    changedTouches = []

    constructor(type) {
        super(type)

        this.target = window.canvas
        this.currentTarget = window.canvas
    }
}

function eventHandlerFactory(type) {
    return (rawEvent) => {
        const event = new TouchEvent(type)

        event.changedTouches = rawEvent.changedTouches
        event.touches = rawEvent.touches
        event.targetTouches = Array.prototype.slice.call(rawEvent.touches)
        event.timeStamp = rawEvent.timeStamp

        document.dispatchEvent(event)
    }
}

wx.onTouchStart(eventHandlerFactory('touchstart'))
wx.onTouchMove(eventHandlerFactory('touchmove'))
wx.onTouchEnd(eventHandlerFactory('touchend'))
wx.onTouchCancel(eventHandlerFactory('touchcancel'))
