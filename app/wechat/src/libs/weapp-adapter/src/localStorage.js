const localStorage = {
    get length() {
        const { keys } = wx.getStorageInfoSync()
        return keys.length
    },

    key(n) {
        const { keys } = wx.getStorageInfoSync()

        return keys[n]
    },

    getItem(key) {
        const value = wx.getStorageSync(key);
        return value === "" ? null : value;
    },

    setItem(key, value) {
        return wx.setStorageSync(key, value)
    },

    removeItem(key) {
        wx.removeStorageSync(key)
    },

    clear() {
        wx.clearStorageSync()
    }
}

export default localStorage
