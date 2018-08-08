#pragma once

#include <CEGUI/include/CEGUIAdapter.h>

namespace Examples
{
	/**
	 *  CEGUI屏幕适配器
	 *  2015年4月15日 17:39:08
	 *  moved by 王晓波 --> author songjianbang
	 */
	class UIAdapter :public CEGUI::IAdapter
	{
	public:
		void Initialize(int screenW, int screenH);
		int GetLogicWidth(){ return m_logicUIW; }
		int GetLogicHeight(){ return m_logicUIH; }
		int GetScreenWidth(){ return m_screenUIW; }
		int GetScreenHeight(){ return m_screenUIH; }
		int GetDisplayOffsetX(){ return m_displayUIX; }
		int GetDisplayOffsetY(){ return m_displayUIY; }
		int GetDisplayWidth(){ return m_displayUIW; }
		int GetDisplayHeight(){ return m_displayUIH; }
		float GetRateW(){ return m_rateW; };
		float GetRateH(){ return m_rateH; };

	private:
		int m_logicW, m_logicH;
		int m_screenW, m_screenH;
		int m_displayX, m_displayY;
		int m_displayW, m_displayH;

		int m_displayUIX, m_displayUIY;
		int m_displayUIW, m_displayUIH;
		int m_screenUIW, m_screenUIH;
		int m_logicUIW, m_logicUIH;

		float m_rateW, m_rateH;
  };
}