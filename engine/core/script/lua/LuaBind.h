#pragma once

struct lua_State;

namespace Echo
{
	class LuaBind
	{
	public:
		LuaBind();
		~LuaBind();

		// get instance
		static LuaBind* instance();

		// destory
		static void destroy();

	private:
		// out error
		void outputError(lua_State* state);
	};
}