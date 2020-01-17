import EventTarget from './EventTarget.js'

const _url = new WeakMap()
const _method = new WeakMap()
const _requestHeader = new WeakMap()
const _responseHeader = new WeakMap()
const _requestTask = new WeakMap()

const fs = wx.getFileSystemManager()

function _triggerEvent(type, event = {}) {
    event.target = event.target || this

    if (typeof this[`on${type}`] === 'function') {
        this[`on${type}`].call(this, event)
    }
}

function _changeReadyState(readyState, event = {}) {
    this.readyState = readyState

    event.readyState = readyState;

    _triggerEvent.call(this, 'readystatechange', event)
}

function _isRelativePath(url) {
    return !(/^(http|https|ftp|wxfile):\/\/.*/i.test(url));
}

export default class XMLHttpRequest extends EventTarget {
    // TODO 没法模拟 HEADERS_RECEIVED 和 LOADING 两个状态
    static UNSEND = 0
    static OPENED = 1
    static HEADERS_RECEIVED = 2
    static LOADING = 3
    static DONE = 4

    /*
     * TODO 这一批事件应该是在 XMLHttpRequestEventTarget.prototype 上面的
     */
    onabort = null
    onerror = null
    onload = null
    onloadstart = null
    onprogress = null
    ontimeout = null
    onloadend = null

    onreadystatechange = null
    readyState = 0
    response = null
    responseText = null
    responseType = ''
    responseXML = null
    status = 0
    statusText = ''
    upload = {}
    withCredentials = false

    constructor() {
        super();

        _requestHeader.set(this, {
            'content-type': 'application/x-www-form-urlencoded'
        })
        _responseHeader.set(this, {})
    }

    abort() {
        const myRequestTask = _requestTask.get(this)

        if (myRequestTask) {
            myRequestTask.abort()
        }
    }

    getAllResponseHeaders() {
        const responseHeader = _responseHeader.get(this)

        return Object.keys(responseHeader).map((header) => {
            return `${header}: ${responseHeader[header]}`
        }).join('\n')
    }

    getResponseHeader(header) {
        return _responseHeader.get(this)[header]
    }

    open(method, url /* async, user, password 这几个参数在小程序内不支持*/ ) {
        _method.set(this, method)
        _url.set(this, url)
        _changeReadyState.call(this, XMLHttpRequest.OPENED)
    }

    overrideMimeType() {}

    send(data = '') {
        if (this.readyState !== XMLHttpRequest.OPENED) {
            throw new Error("Failed to execute 'send' on 'XMLHttpRequest': The object's state must be OPENED.")
        } else {
            const url = _url.get(this)
            const header = _requestHeader.get(this)
            const responseType = this.responseType

            const relative = _isRelativePath(url)
            let encoding;

            if (responseType === 'arraybuffer') {
                // encoding = 'binary'
            } else {
                encoding = 'utf8'
            }

            delete this.response;
            this.response = null;

            const onSuccess = ({ data, statusCode, header }) => {
                statusCode = statusCode === undefined ? 200 : statusCode;
                if (typeof data !== 'string' && !(data instanceof ArrayBuffer)) {
                    try {
                        data = JSON.stringify(data)
                    } catch (e) {
                        data = data
                    }
                }

                this.status = statusCode
                if (header) {
                    _responseHeader.set(this, header)
                }
                _triggerEvent.call(this, 'loadstart')
                _changeReadyState.call(this, XMLHttpRequest.HEADERS_RECEIVED)
                _changeReadyState.call(this, XMLHttpRequest.LOADING)

                this.response = data

                if (data instanceof ArrayBuffer) {
                    Object.defineProperty(this, 'responseText', {
                        enumerable: true,
                        configurable: true,
                        get: function() {
                            throw "InvalidStateError";
                        }
                    });
                } else {
                    this.responseText = data
                }
                _changeReadyState.call(this, XMLHttpRequest.DONE)
                _triggerEvent.call(this, 'load')
                _triggerEvent.call(this, 'loadend')
            }

            const onFail = ({ errMsg }) => {
                // TODO 规范错误

                if (errMsg.indexOf('abort') !== -1) {
                    _triggerEvent.call(this, 'abort')
                } else {
                    _triggerEvent.call(this, 'error', {
                        message: errMsg
                    })
                }
                _triggerEvent.call(this, 'loadend')

                if (relative) {
                    // 用户即使没监听error事件, 也给出相应的警告
                    console.warn(errMsg)
                }
            }

            if (relative) {
                var options = {
                    'filePath': url,
                    'success': onSuccess,
                    'fail': onFail
                }
                if (encoding) {
                    options['encoding'] = encoding;
                }
                fs.readFile(options)
                return
            }

            wx.request({
                data,
                url: url,
                method: _method.get(this),
                header: header,
                responseType: responseType,
                success: onSuccess,
                fail: onFail
            })
        }
    }

    setRequestHeader(header, value) {
        const myHeader = _requestHeader.get(this)

        myHeader[header] = value
        _requestHeader.set(this, myHeader)
    }

    addEventListener(type, listener) {
        if (typeof listener !== 'function') {
            return;
        }

        this['on' + type] = (event = {}) => {
            event.target = event.target || this
            listener.call(this, event)
        }
    }

    removeEventListener(type, listener) {
        if (this['on' + type] === listener) {
            this['on' + type] = null;
        }
    }
}
