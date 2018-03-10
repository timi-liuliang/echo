#define TRACKLISTENPORT 20000

// 获取模块表
#define TMSG_GETMODULELIST	100
// 要求加载模块
#define TMSG_LOADMODULE		101
// 要求卸载模块
#define TMSG_UNLOADMODULE	102
// 获取符号表
#define TMSG_GETSYMLOADLIST 102
// 追踪某个调用
#define TMSG_TRACKCALL		103
// 取消对某个调用的追踪
#define TMSG_UNDO_TRACKCALL 104

// 返回模块列表
#define RMSG_MODULELIST		200
// 返回符号列表
#define RMSG_SYMLOADLIST	201
// 返回追踪数据
#define RMSG_FRAMEINFO		202
// 返回监听结果
#define RMSG_LISTENRESULT	203
// 取消监听结果
#define RMSG_UNDO_LISTENRESULT 204

