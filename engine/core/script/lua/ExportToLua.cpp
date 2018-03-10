#include "ExportToLua.h"
#include "Engine/core/main/Root.h"
//#include <luaex.h>

namespace Echo
{
	// 注册c++函数供lua调用
	void registerInterfaceToLua()
	{
		//luaex::LuaEx* luaEx = Root::instance()->getLuaEx();
		//if (!luaEx)
		//	return;

		//// 日志接口注册
		//{
		//	// 日志类,日志类函数注册
		//	luaEx->register_class("LogManager");
		//	luaEx->register_function<LogManager, const char*>("LogManager", "error", &LogManager::error);
		//	luaEx->register_function<LogManager, const char*>("LogManager", "warning", &LogManager::warning);
		//	luaEx->register_function<LogManager, const char*>("LogManager", "info", &LogManager::info);

		//	// 日志对象注册
		//	luaEx->register_object("LogManager", "log", LogManager::instance());
		//}

		//// 组件接口注册
		//{
		//	luaEx->register_class("GameObject");
		//	luaEx->register_function<GameObject, const char*>("GameObject", "activeComponent", &GameObject::activeComponent);
		//}
	}
}