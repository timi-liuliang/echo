#pragma once

#include <Cegui/include/CEGUI.h>
#include <Cegui/include/CEGUISystem.h>
#include <Cegui/include/CEGUIAdapter.h>
#include <Cegui/include/CEGUIPfsResourceProvider.h>
#include <Cegui/include/RendererModules/Lord/CEGUILordRenderer.h>
#include <Cegui/include/CEGUIEvent.h>
#include "UIAdapter.h"

namespace Examples
{
	/**
	 * 例子界面管理器
	 */
	class UIManager
	{
	public:
		UIManager();
		~UIManager();

		// 初始化
		void initialize();

		// 更新
		void tick( int delta);

		// 窗口大小改变
		void onSize(int screenW, int screenH);

		// 获取根窗口
		CEGUI::Window* getRootWindow() { return m_rootWindow; }

	private:
		// 设置资源目录
		void initResourceDirectoriesAndGroups();

		// 开始模态
		bool onStartModalState(const CEGUI::EventArgs& e);

		// 加载UI界面前调用
		bool handleBeforeLoadlayout(const CEGUI::EventArgs& e);

	private:
		CEGUI::CEGUIRenderer*		m_uiRenderer;			// CEGUI渲染器
		Examples::UIAdapter			m_adapter;				// 适配器
		CEGUI::ResourceProvider*	m_uiResourceProvider;	// 资源管理器
		CEGUI::Window*              m_rootWindow;			// 根窗口
	};
}