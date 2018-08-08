#include "UIAdapter.h"

namespace Examples
{
	static const float c_aspec_min = 1.0f;
	static const float c_aspec_max = 2.0f;
	static const int c_width_min = 960;
	static const int c_height_min = 640;

	static const int c_width_max = 1280;
	static const int c_height_max = 800;

	void UIAdapter::Initialize(int screenW, int screenH)
	{
		m_screenW = screenW;
		m_screenH = screenH;

		if (m_screenW < c_width_min || m_screenH < c_height_min)
		{
			float w = (float)c_width_min / m_screenW;
			float h = (float)c_height_min / m_screenH;
			if (w >= h)
			{
				m_logicW = static_cast<int>(w*m_screenW);
				m_logicH = static_cast<int>(w*m_screenH);
			}
			else
			{
				m_logicW = static_cast<int>(h*m_screenW);
				m_logicH = static_cast<int>(h*m_screenH);
			}
		}
		else if (m_screenW > c_width_max || m_screenH > c_height_max)
		{
			float w = (float)m_screenW / c_width_max;
			float h = (float)m_screenH / c_height_max;
			if (w >= h)
			{
				m_logicW = c_width_max;
				m_logicH = static_cast<int>(m_screenH / w);
			}
			else
			{
				m_logicW = static_cast<int>(m_screenW / h);
				m_logicH = c_height_max;
			}
		}
		else
		{
			m_logicW = m_screenW;
			m_logicH = m_screenH;
		}
		m_displayW = m_screenW;
		m_displayH = m_screenH;

		m_screenUIW = m_screenW;
		m_screenUIH = m_screenH;

		m_logicUIW = m_logicW;
		m_logicUIH = m_logicH;

		m_displayUIW = m_screenW;
		m_displayUIH = m_screenH;

		m_rateW = float(m_screenW) / float(m_logicW);
		m_rateH = float(m_screenH) / float(m_logicH);

		m_displayUIX = 0;
		m_displayUIY = 0;
	};
}