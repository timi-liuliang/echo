const { screenWidth, screenHeight, devicePixelRatio } = wx.getSystemInfoSync()

export const innerWidth = screenWidth
export const innerHeight = screenHeight
export { devicePixelRatio }
export const screen = {
    width: screenWidth,
    height: screenHeight,
    availWidth: innerWidth,
    availHeight: innerHeight,
    availLeft: 0,
    availTop: 0,
}
export const scrollX = 0
export const scrollY = 0
export const ontouchstart = null
export const ontouchmove = null
export const ontouchend = null

export performance from './performance'
