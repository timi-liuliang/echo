#pragma once

#include <string>
#include <Engine/Core.h>
#include <Engine/core/main/Root.h>
#include <Engine/core/Scene/Scene_Manager.h>
#include <Engine/modules/Model/Mesh.h>
#include <Engine/modules/Audio/FMODStudio/FSAudioManager.h>

#define  FPSINFONUM 6

namespace Studio
{
	class RenderWindow; 
	class FBXManager;
	class EchoEngine : public Echo::Singleton<EchoEngine>
	{
	public:
		EchoEngine();
		~EchoEngine();

		// 初始化
		bool Initialize( HWND hwnd);

		// 每帧渲染
		void Render( unsigned int elapsedTime, bool isRenderWindowVisible);

		// 修改窗口大小
		void Resize(int cx, int cy);

		// 设置project
		static bool SetProject( const char* projectFile);

		// 卸载
		void Release();

		// 获取FBX格式管理器
		FBXManager* GetFBXManager() { return m_fbxManager; }

	public:
		// 设置当前编辑结点
		void setCurrentEditNode(Echo::Node* node) { m_currentEditNode = node; }

		// 获取当前编辑结点
		Echo::Node* getCurrentEditNode() { return m_currentEditNode; }

	public:
		// 预览声音
		virtual void previewAudioEvent( const char* audioEvent);

		// 停止正在预览的声源
		virtual void stopCurPreviewAudioEvent();

		//设置显示或隐藏背景网格
		virtual void setBackGridVisibleOrNot(bool showFlag);

		//调整背景网格的参数
		virtual void ResizeBackGrid(int linenums,float lineGap);

		//获得背景网格的相关参数
		virtual void GetBackGridParameters(int* linenums,float* lineGap);

		// 获取模型半径
		float GetMeshRadius();

		// 保存缩略图
		void SaveSceneThumbnail(bool setCam = true);

		// 场景相关
	public:
		// 切换场景
		void ChangeScene( const char* sceneName, bool isGameMode=false);

		// 保存当前编辑场景
		void SaveScene();

		// 设置FPS是否显示
		void SetIsFPSShow(bool setting){ m_isShowFps = setting; }

	private:
		// 初始化背景网格
		void InitializeBackGrid();

		// 初始化FPS显示
		void InitFPSShow();

		// 更新FPS显示
		void UpdateFPS();

		// 更新字体节点
		void upDateFpsNode(int index);

	protected:
		Echo::Log*				m_log;				// 日志
		Echo::SceneManager*		m_sceneMgr;			// 场景管理器
		Echo::Node*				m_currentEditNode;	// 当前编辑场景

		//Echo::node*			m_backGridNode;	// 背景网格用场景结点
		//Echo::DebugDisplayItem*	m_backGrid;		// 背景网格
		//int						m_gridNum;		// 网格的数量
		//float					m_gridGap;		// 网格间距

		Echo::Node *		m_pCameraAxis;

		static std::string		m_projectFile;		// 项目名称
		static RenderWindow*	m_renderWindow;	// 渲染窗口

		FBXManager*				m_fbxManager;		// fbx格式管理器

		Echo::AudioSourceID		m_curPlayAudio;		// 当前播放音频

		//FontRenderManager*  m_FontRenderManager; // 字体渲染  
		bool					m_isShowFps;		 // 是否显示FPS	
		Echo::Node*				m_FpsPrarentNode;	 // FPS父节点
		Echo::array<Echo::Node*,FPSINFONUM>	m_pFpsNodes;	 // FPS显示，挂载点
		//Echo::array<FontCNRender*, FPSINFONUM>		m_pTextRenders;	 // 字体集合
		bool				m_isManualUpdateEngine; 
	};
}
