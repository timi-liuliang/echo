#pragma once

#include "PostImageEffect.h"

namespace rapidxml
{
	template<typename Ch> class xml_node;
}

namespace Echo
{
	class WeatherLightningImageEffect :public PostImageEffect
	{
	public:
		WeatherLightningImageEffect(const String& name);
		~WeatherLightningImageEffect();

		void weatherLightningReset();
		void adjustWeatherLightningStatus();
		void playWeatherSystemSound(const Echo::String& path);

		void render();
		void setLogicCalcParameter(const String& name, const String& value);
		String getLogicCalcParameter(const String& name);
	private:
		bool m_bWeatherLightingNeedReset;	//是否需要重置全局闪电参数
		bool m_bWeatherSingleLightiningEnable;	//单次闪电是否开启
		bool m_bWeatherLIghtiningSoundEnable;	//是否需要播放闪电音效
		float m_weatherLightingTotalTime;	//单次闪电总持续时间
		float m_weatherLightingElapsedTime;	//目前流逝时间
		Vector4 m_weatherLightingDurationTimes;	//单次闪电各分段的持续时间
		float m_weatherLightingStartTime;	//闪电系统开始时间
		float m_weatherLightningIntervalTime;	//全局闪电间隔
		Echo::String m_lightningSoundPath;	//闪电音效的路径

	};
	
}  // namespace Echo
