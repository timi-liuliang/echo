#pragma once

#include <engine/core/Util/Array.hpp>
#include "Light.h"

namespace Echo
{
	/**
	 * 光源阵列
	 */
	class LightArray
	{
	public:
		LightArray();
		~LightArray();

		// 获取名称
		const char* getName() const { return m_name.c_str(); }

		// 更新
		void update();

		void updatePointLightPos( const Vector4& pos );

		// 获取光源数量
		int getLightNum() { return static_cast<int>(m_lights.size()); }

		// 获取光源
		Light* getLight(int idx) { return m_lights[idx]; }

		// 获取光源信息
		void* getLightInfo() { return m_info.data(); }

		// 获取光源数据
		void* getLightData() { return m_data.data(); }

		void* getPBRLightPos() { return m_pbrLightPos.data(); }
		void* getPBRLightDir() { return m_pbrLightDir.data(); }
		void* getPBRLightSopt() { return m_pbrLightSpot.data(); }
		void* getPBRLightParam() { return m_pbrLightParam.data(); }
		void* getPBRLightColor() { return m_pbrLightColor.data(); }

		// 添加光源
		Light* addLight(LightType type);

		// 删除光源
		void deleteLight(int idx);

		// 删除所有光源
		void deleteAllLights();

		// 设置脏标记
		void setDirty() { m_isDirty = true; }

		bool isDirty() { return m_isDirty; }

		// 加载
		void load(const char* fileName);

		// 保存
		void save(const char* fullPath);

	private:
		String						m_name;		// 文件名称
		array<LightType, 8>			m_info;		// 光源类型信息
		array<Vector4, 16>			m_data;		// 光源数据
		vector<Light*>::type		m_lights;	// 光源
		bool						m_isDirty;	// 脏标记

		vector<Vector4>::type		m_pbrLightPos;
		vector<Vector3>::type		m_pbrLightDir;
		vector<Vector3>::type		m_pbrLightColor;
		vector<Vector3>::type		m_pbrLightSpot;
		vector<Vector3>::type		m_pbrLightParam;
	};
}