#include "EngineTimeController.h"
#include <engine/core/Math/Math.h>
#include <engine/core/Util/StringUtil.h>
#include <engine/core/Util/AssertX.h>
#include <engine/core/Base/Function.h>

namespace Echo
{
	static EngineTimeController*	g_timeController = nullptr;

	// 构造函数
	EngineTimeController::EngineTimeController()
	{
		clear();
	}

	// 获取实例
	EngineTimeController* EngineTimeController::instance()
	{
		if (!g_timeController)
		{
			g_timeController = EchoNew(EngineTimeController);
		}

		return g_timeController;
	}

	// 销毁
	void EngineTimeController::destroy()
	{
		EchoSafeDelete(g_timeController, EngineTimeController);
	}

	// 获取当前时间倍率
	float EngineTimeController::getSpeed(int t)
	{
		return (t & m_type) ? m_speed : 1.f;
	}

	// 清空
	void EngineTimeController::clear()
	{
		m_elapsed = 0.f;
		m_keyIdx = 0;
		m_speed = 1.f;
		m_type = ET_UI | ET_SOUND | ET_SCENE;
		m_keys.clear();
	}

	// 设置任务
	void EngineTimeController::setKeys(const char* keys)
	{
		clear();

		StringArray strArray = StringUtil::Split(keys, ";");
		for (const String& str : strArray)
		{
			StringArray valueArray = StringUtil::Split(str, ",");

			Key key;
			key.m_time = StringUtil::ParseFloat(valueArray[0]);
			key.m_speed = StringUtil::ParseFloat(valueArray[1]);
			m_keys.push_back(key);
		}

		EchoAssert( m_keys.size()!=1);
	}

	void EngineTimeController::setExclusive(ui32 flag)
	{
		m_type &= ~flag;
	}

	// 更新(秒)
	void EngineTimeController::update(float elapsed)
	{
		if (!m_keys.empty())
		{
			m_elapsed += elapsed;

			// 移动帧
			for (size_t i = m_keyIdx; i < m_keys.size(); i++)
			{
				if (m_keys[i].m_time < m_elapsed)
					m_keyIdx = i;
				else
					break;
			}

			// 是否满足清空条件
			if (m_keyIdx + 1 == m_keys.size())
			{
				clear();
				return;
			}

			// 插值计算帧率
			const Key& firstKey = m_keys[m_keyIdx];
			const Key& nextKey = m_keys[m_keyIdx + 1];
			auto delta = nextKey.m_time - firstKey.m_time;
			if (Math::Abs(delta) < Math::EPSILON)
			{
				m_speed = firstKey.m_speed;
			}
			else
			{
				float radio = (m_elapsed - firstKey.m_time) / delta;
				m_speed = firstKey.m_speed + (nextKey.m_speed - firstKey.m_speed) * radio;
			}
		}
		else
		{
			m_speed = 1.f;
		}
	}
}