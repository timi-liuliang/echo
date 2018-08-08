//
// Lua Example
//
// 2016年6月1日 10:51:34
//

#pragma once
#include "Frame/Example.h"

struct lua_State; 
typedef int(*lua_CFunction) (lua_State *L);

namespace Examples
{
	class LuaModule : public Example
	{
		LuaModule(const LuaModule& rhs);
		LuaModule& operator=(const LuaModule& rhs);

	public: 
		LuaModule();
		virtual ~LuaModule();

	public: 
		// 帮助信息
		virtual const LORD::String getHelp();

		// 初始化
		virtual bool initialize();

		// 更新
		virtual void tick(LORD::ui32 elapsedTime);

		// 销毁
		virtual void destroy();

		// 键盘事件 
		virtual void keyboardProc(LORD::ui32 keyChar, bool isKeyDown) override;

		// 鼠标事件
		virtual void mouseLBProc(LORD::i32 xpos, LORD::i32 ypos) override;

		// 鼠标事件
		virtual void mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos) override;

		// 滚轮事件
		virtual void mouseWhellProc(LORD::i32 parma) override; 

	private:
		void addLuaLoader(lua_State* L, lua_CFunction func); 

		bool doLua(lua_State* L, const LORD::String& fileName); 

	private:
		lua_State*							m_pState; 
	};
}