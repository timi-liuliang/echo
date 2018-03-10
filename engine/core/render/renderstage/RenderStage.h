#pragma once

#include "engine/core/Util/Singleton.h"
#include <engine/core/Memory/MemManager.h>
#include <engine/core/render/render/RenderState.h>
#include <engine/core/render/render/RenderInput.h>

namespace Echo
{
	// 渲染阶段ID
	enum RenderStageID
	{
		RSI_EditorBackground,		// 编辑器背景
		RSI_Coverage,				// 遮蔽图
		RSI_LDRSceneColor,			// 低动态范围
		RSI_HDRSceneColor,			// 高动态范围
		RSI_PostProcess,			// 后处理阶段
		RSI_AfterPostProcess,		// 后处理完成后阶段
		RSI_Reflective,				// 镜面反射
		RSI_ShadowMap,				// 渲染阴影图
		RSI_WaterRefectMap,			// 水面反射图
		RSI_Glow,
		RSI_Spine,					// 2D骨骼动画
		RSI_UI,						// 界面
		RSI_EditorFrontground,		// 界面字体
		RSI_End
	};

	/**
	* 渲染阶段
	*/
	class RenderStageManager;
	class RenderStage
	{
		friend class RenderStageManager;
	public:
		RenderStage(RenderStageID stageName);
		virtual ~RenderStage();

		// 设置是否可用
		virtual void setEnable(bool _enable) { m_bEnable = _enable; }

		// 是否可用
		bool getEnable() const { return m_bEnable; }

		// 获取ID
		const RenderStageID getID() const { return m_rsID; }

		// 渲染
		virtual void render() = 0;

		// 初始化
		virtual bool initialize() = 0;

		// 销毁
		virtual void destroy() = 0;

	protected:
		bool			m_bEnable;				// 是否可用
		bool			m_bInitialized;			// 是否已初始化
		RenderStageID	m_rsID;					// ID
	};
}