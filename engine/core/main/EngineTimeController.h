#pragma once

#include "engine/core/Memory/MemManager.h"

namespace Echo
{
	/**
	 * 时间控制器
	 */
	class EngineTimeController
	{
	public:
		enum SlowDownType
		{
			ET_SOUND = 0x00000001,
			ET_UI = 0x00000002,
			ET_SCENE = 0x00000004
		};
		// 帧
		struct Key
		{
			float	m_time;		// 时间
			float	m_speed;	// 速率
		};
		typedef vector<Key>::type Keys;

	public:
		// 获取实例
		static EngineTimeController* instance();

		// 销毁
		static void destroy();

		// 获取当前时间倍率
		float getSpeed(int t = ET_UI | ET_SOUND | ET_SCENE);

		// 设置任务
		void setKeys( const char* keys);

		void setExclusive(ui32 flag);

		// 更新(秒)
		void update( float elapsed);

		// 清空
		void clear();

	private:
		EngineTimeController();
		~EngineTimeController() {}

	private:
		float	m_elapsed;		// 处理时长
		Keys	m_keys;			// 帧列
		ui32	m_keyIdx;		// 当前帧
		float	m_speed;		// 当前速率
		int		m_type;
	};
}