#include "WeatherLightningImageEffect.h"
#include "Engine/modules/Audio/FMODStudio/FSAudioManager.h"
#include "RenderTargetManager.h"

namespace Echo
{

	WeatherLightningImageEffect::WeatherLightningImageEffect(const String& name)
		: PostImageEffect(name)
		, m_bWeatherLightingNeedReset(true)
		, m_bWeatherSingleLightiningEnable(false)
		, m_bWeatherLIghtiningSoundEnable(false)
		, m_weatherLightingStartTime(0)
		, m_weatherLightingElapsedTime(0)
		, m_weatherLightingTotalTime(0)
		, m_weatherLightingDurationTimes(0, 0, 0, 0)
		, m_weatherLightningIntervalTime(60)
		, m_lightningSoundPath("")
	{

	}

	WeatherLightningImageEffect::~WeatherLightningImageEffect()
	{

	}
	void WeatherLightningImageEffect::render()
	{
		if (m_enable)
		{
			setParameter("elapsedTime", Echo::Vector4(m_weatherLightingElapsedTime));
			if (m_bWeatherLightingNeedReset)
			{
				weatherLightningReset();
			}

			adjustWeatherLightningStatus();

			if (m_bWeatherSingleLightiningEnable)
			{
				PostImageEffect::render();
			}
		}
		else
		{
			m_bWeatherLightingNeedReset = true;
		}
	}

	void WeatherLightningImageEffect::setLogicCalcParameter(const String& name, const String& value)
	{
		if (name == "intervalTime")
		{
			m_weatherLightningIntervalTime = StringUtil::ParseFloat(value);
		}
		else if (name == "soundPath")
		{
			m_lightningSoundPath = value;
		}
		else if (name == "durationTimes")
		{
			m_weatherLightingDurationTimes = StringUtil::ParseVec4(value);
		}
	}

	String WeatherLightningImageEffect::getLogicCalcParameter(const String& name)
	{
		if (name == "intervalTime")
		{
			return StringUtil::ToString(m_weatherLightningIntervalTime);
		}
		else if (name == "soundPath")
		{
			return m_lightningSoundPath;
		}
		
		return "";
	}

	void WeatherLightningImageEffect::weatherLightningReset()
	{
		m_weatherLightingStartTime = Time::instance()->getMilliseconds() / 1000.f;
		m_weatherLightingElapsedTime = 0;
		m_bWeatherLightingNeedReset = false;
	}

	void WeatherLightningImageEffect::adjustWeatherLightningStatus()
	{
		m_weatherLightingTotalTime = m_weatherLightingDurationTimes.x + m_weatherLightingDurationTimes.y + m_weatherLightingDurationTimes.z + m_weatherLightingDurationTimes.w;
		m_weatherLightingElapsedTime = Time::instance()->getMilliseconds() / 1000.f - m_weatherLightingStartTime;
		if (m_weatherLightingElapsedTime < m_weatherLightingTotalTime) //播放单次闪电
		{
			if (!m_bWeatherLIghtiningSoundEnable)
			{
				m_bWeatherLIghtiningSoundEnable = true;;
				playWeatherSystemSound(m_lightningSoundPath);
			}
			m_bWeatherSingleLightiningEnable = true;
		}
		else if (m_weatherLightingElapsedTime < m_weatherLightningIntervalTime) //播放闪电结束等待下一次播放
		{
			m_bWeatherSingleLightiningEnable = false;
			m_bWeatherLIghtiningSoundEnable = false;
		}
		else //重置状态，开启下一次播放
		{
			m_weatherLightingStartTime = Time::instance()->getMilliseconds() / 1000.f;
			m_weatherLightingElapsedTime = 0;
		}
	}

	void WeatherLightningImageEffect::playWeatherSystemSound(const Echo::String& path)
	{
		if (path.empty())
		{
			return;
		}
		Echo::AudioSource::Cinfo cinfo;
		cinfo.m_name = path;
		cinfo.m_is3DMode = false;
		FSAudioManager::instance()->createAudioSource(cinfo);
	}
}