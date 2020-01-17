import { noop } from './util/index.js'

export default class Event {

    cancelBubble = false
    cancelable = false
    target = null
    currentTarget = null
    preventDefault = noop
    stopPropagation = noop

    constructor(type) {
        this.type = type
        this.timeStamp = Date.now()
    }
}
