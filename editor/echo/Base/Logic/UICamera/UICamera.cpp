#include "UICamera.h"

namespace Studio
{
	class CAdapter
	{
	private:
		int m_logicW, m_logicH;
		int m_screenW, m_screenH;
		int m_displayX, m_displayY;
		int m_displayW, m_displayH;

		int m_displayUIX, m_displayUIY;
		int m_displayUIW, m_displayUIH;
		int m_screenUIW, m_screenUIH;
		int m_logicUIW, m_logicUIH;

		float m_rateW,m_rateH;
	public:
		void Initialize(int screenW, int screenH);

		int GetLogicWidth(){return m_logicUIW;}
		int GetLogicHeight(){return m_logicUIH;}
		int GetScreenWidth(){return m_screenUIW;}
		int GetScreenHeight(){return m_screenUIH;}
		int GetDisplayOffsetX(){return m_displayUIX;}
		int GetDisplayOffsetY(){return m_displayUIY;}
		int GetDisplayWidth(){return m_displayUIW;}
		int GetDisplayHeight(){return m_displayUIH;}
		float GetRateW(){ return m_rateW; };
		float GetRateH(){ return m_rateH; };
	};

	void CAdapter::Initialize(int screenW, int screenH)
	{
		m_screenW = screenW;
		m_screenH = screenH;

		static const float c_aspec_min = 1.0f;
		static const float c_aspec_max = 2.0f;
		static const int c_width_min = 960;
		static const int c_height_min = 640;

		static const int c_width_max = 1280;
		static const int c_height_max = 800;

		if (m_screenW < c_width_min || m_screenH < c_height_min)
		{
			float w = (float)c_width_min/m_screenW;
			float h = (float)c_height_min/m_screenH;
			if (w >= h)
			{
				m_logicW = static_cast<int>(w*m_screenW);
				m_logicH = static_cast<int>(w*m_screenH);
			}else
			{
				m_logicW = static_cast<int>(h*m_screenW);
				m_logicH = static_cast<int>(h*m_screenH);
			}
		}
		else if(m_screenW > c_width_max || m_screenH > c_height_max)
		{
			float w = (float)m_screenW/c_width_max;
			float h = (float)m_screenH/c_height_max;
			if (w >= h)
			{
				m_logicW = c_width_max;
				m_logicH = static_cast<int>(m_screenH / w);
			}else
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

		m_rateW = float(m_screenW)/float(m_logicW);
		m_rateH = float(m_screenH)/float(m_logicH);

		m_displayUIX=0;
		m_displayUIY=0;
	};

	// 构造函数
	UICamera::UICamera() 
		: Camera(Camera::PM_UI) 
	{
	}

	// 析构函数
	UICamera::~UICamera()
	{

	}

	// 设置屏幕宽高
	void UICamera::setWidthHeight( Echo::ui32 width, Echo::ui32 height)
	{
		CAdapter adapter;
		adapter.Initialize( width, height);

		//setUp( -Echo::Vector3::UNIT_Y);
		setWidth( adapter.GetLogicWidth());
		setHeight( adapter.GetLogicHeight());

		update();
	}

	// 更新
	void UICamera::update()
	{
		m_matVP.identity();

		float doubleInvScreenWidth = 2.f / m_width;
		float doubleInvScreenHeight= 2.f / m_height;
		float invDepth			   = 1.f / 500.f;

		m_matVP.scaleReplace( doubleInvScreenWidth, -doubleInvScreenHeight, invDepth);
		m_matVP.translate( Echo::Vector3( -1.f, 1.f, 0.5f));
	}

	// UI逻辑大小映射
	void MappingUILogicWH( Echo::ui32 screenWidth, Echo::ui32 screeHeight, Echo::ui32& oLogicWidth, Echo::ui32& oLogicHeight)
	{
		CAdapter adapter;
		adapter.Initialize( screenWidth, screeHeight);

		oLogicWidth = adapter.GetLogicWidth();
		oLogicHeight= adapter.GetLogicHeight();
	}
}