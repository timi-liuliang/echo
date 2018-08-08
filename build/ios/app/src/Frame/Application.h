#pragma once

#include <Foundation/Util/LogManager.h>
#include "Example.h"
#include "UIManager.h"

namespace Examples
{
	/**
	 * 执行程序
	 */
	class Application : public CEGUI::EventSet
	{
	public:
		Application();
		~Application();

		// for Android: set resource dir.
		void setAssetPath(const char* path, const char* internal_path); 

		// 启动执行
		void start(LORD::i32 width = 0, LORD::i32 height = 0);

		void end();

		// 更新
		void tick(LORD::ui32 elapsedTime);

		// 开启示例
		void startExample(LORD::ui32 index);

		// 根据索引获取示例工厂
		ExampleFactory* getFactoryByIdx(int idx);

	public:
		// 按键消息
		void keyboardProc( LORD::ui32 keyChar, bool isKeyDown);

		// 鼠标滚轮消息
		void mouseWhellProc( LORD::i32 parma);

		// 鼠标消息处理
		void mouseProc( LORD::i32 xpos, LORD::i32 ypos);

		// 鼠标左键消息
		void mouseLBProc( LORD::i32 xpos, LORD::i32 ypos, bool isUp = false);

		// 鼠标移动消息
		void mouseMoveProc( LORD::i32 xpos, LORD::i32 ypos);

		// 启动下一示例
		bool nextExample(const CEGUI::EventArgs& args);

		// 启动上一示例
		bool lastExample(const CEGUI::EventArgs& args);

		// 重启当前示例
		bool restartExample(const CEGUI::EventArgs& args);

		void flushRender();

	private:
		// 初始化引擎
		void initEngine();

		// 初始化UI
		void initUI();

		// 重置窗口大小
		void resize(int cx, int cy);

		// 检测窗口大小变化
		void checkScreenSize();

	private:
		LORD::i32					m_screenWidth;		// 屏幕宽
		LORD::i32					m_screenHeight;		// 屏幕高
		LORD::LogDefault*			m_log;				// 日志实现
		LORD::ui32					m_curExampleIdx;	// 示例索引
		Example*					m_curExample;		// 当前示例
		UIManager*					m_uiManager;		// 界面管理
		CEGUI::Window*				m_selectWindow;		// 例子选择窗口
		CEGUI::Window*				m_circulateWindow;	// 循环选择窗口

		LORD::String				m_path; 
		LORD::String				m_internalPath; 
	};

	/**
	 * 消息回调
	 */
	class CEGUIFunctor
	{
	public:
		CEGUIFunctor(int id):m_id(id) {}
		~CEGUIFunctor() {}

		// 运算符重载
		bool operator()(const CEGUI::EventArgs& args) const;

	private:
		int	m_id;
	};
}