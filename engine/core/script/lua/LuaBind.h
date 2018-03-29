#pragma once

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
	};
}