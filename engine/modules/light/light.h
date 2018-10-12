#pragma once

#include <engine/core/math/Math.h>

namespace Echo
{
	// 光源类型枚举
	enum LightType
	{
		LT_Unknown = 0,		// 未知
		LT_Ambient,			// 环境光
		LT_Directional,		// 方向光
		LT_Point,			// 点光源
		LT_Spot,			// 聚光灯

		LT_PBRLight,		// PBR使用的混合灯光
	};

	/**
	 * Light
	 */
	struct Light
	{
		LightType			m_type;		// 光源类型
		float				m_intensity;// 光照强度

		Light(LightType type) : m_type(type), m_intensity(1.f) {}
		virtual ~Light() {}

		// 获取类型
		const char* getType();
		const LightType getTypeEnum() { return m_type; }
	};

	/**
	 * 环境光
	 */
	struct AmbientLight : public Light
	{
		Vector4				m_color;	// 颜色

		// 构造函数
		AmbientLight()
			: Light(LT_Ambient)
			, m_color(0.25f, 0.25f, 0.25f, 1.f)
		{}
	};

	/**
	 * 方向光
	 */
	struct DirectionalLight : public Light
	{
		Vector4				m_color;		// 颜色
		Vector4				m_direction;	// 朝向
		Vector3				m_position;

		// 构造函数
		DirectionalLight()
			: Light( LT_Directional)
			, m_color(0.8f, 0.8f, 0.8f, 1.f)
			, m_direction( -0.5774f, -0.5774f, -0.5774f, 1.f)
		{
		}
	};

	/**
	 * 点光源
	 */
	struct PointLight : public Light
	{
		Vector4		m_colorAddFalloff;		// 颜色与衰减值数
		Vector4		m_positionAndInvRadius;	// 位置与半径的逆
		bool		m_followModel;			// 是否跟随模型位置变化

		// 构造函数
		PointLight()
			: Light(LT_Point)
            , m_colorAddFalloff(1.f, 1.f, 1.f, 1.f)
			, m_positionAndInvRadius(0.f, 0.f, 0.f, 0.2f)
			, m_followModel( true )
		{}
	};

	/**
	 * 聚光灯
	 */
	struct SpotLight : public Light
	{
		// 构造函数
		SpotLight()
			: Light(LT_Spot)
		{}
	};
}
