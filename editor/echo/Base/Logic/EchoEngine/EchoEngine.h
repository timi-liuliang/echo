#pragma once

#include <string>
#include <Engine/core/main/Engine.h>
#include <Engine/core/Scene/NodeTree.h>
#include <Engine/core/render/mesh/Mesh.h>
#include <Engine/modules/Audio/FMODStudio/FSAudioManager.h>

#define  FPSINFONUM 6

namespace Studio
{
	class RenderWindow; 
	class EchoEngine
	{
	public:
		~EchoEngine();

		// inst
		static EchoEngine* instance();

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

	public:
		// 设置当前编辑结点
		void setCurrentEditNode(Echo::Node* node) { m_currentEditNode = node; }

		// 获取当前编辑结点
		Echo::Node* getCurrentEditNode() { return m_currentEditNode; }

		// 设置当前结点树存储路径
		void setCurrentEditNodeSavePath(const Echo::String& savePath) { m_currentEditNodeSavePath = savePath; }

		// 获取当前结点树存储路径
		const Echo::String& getCurrentEditNodeSavePath() { return m_currentEditNodeSavePath; }

	public:
		// 预览声音
		virtual void previewAudioEvent( const char* audioEvent);

		// 停止正在预览的声源
		virtual void stopCurPreviewAudioEvent();

		//设置显示或隐藏背景网格
		virtual void setBackGridVisibleOrNot(bool showFlag);

		//调整背景网格的参数
		virtual void ResizeBackGrid();

		//获得背景网格的相关参数
		virtual void GetBackGridParameters(int* linenums,float* lineGap);

		// 获取模型半径
		float GetMeshRadius();

		// 保存缩略图
		void SaveSceneThumbnail(bool setCam = true);

		// 场景相关
	public:
		// new
		void newEditNodeTree();

		// save current node tree
		void saveCurrentEditNodeTree();
		void saveCurrentEditNodeTreeAs(const Echo::String& savePath);

		// 切换场景
		void ChangeScene( const char* sceneName, bool isGameMode=false);

		// 保存当前编辑场景
		void SaveScene();

	private:
		EchoEngine();

		// 初始化背景网格
		void InitializeBackGrid();

	protected:
		Echo::Log*				m_log;						// 日志
		Echo::Node*				m_currentEditNode;			// 当前编辑场景
		Echo::String			m_currentEditNodeSavePath;	// 当前场景存储路径
		Echo::Node*				m_invisibleNodeForEditor;	// 编辑器结点
		Echo::Gizmos*			m_gizmosNodeBackGrid;
		Echo::Node *			m_pCameraAxis;
		static std::string		m_projectFile;				// 项目名称
		static RenderWindow*	m_renderWindow;				// 渲染窗口
		Echo::AudioSourceID		m_curPlayAudio;				// 当前播放音频
	};
}
