#include "EchoEngine.h"
#include <QFileInfo>
#include <QString>
#include <array>
#include <Engine/core/main/Engine.h>
#include <Engine/modules/Navigation/Navigation.h>
#include <interface/RenderTargetManager.h>
#include <interface/RenderTarget.h>
#include "engine/core/Thread/Threading.h"
#include "Studio.h"
#include "RenderWindow.h"
#include <interface/Renderer.h>
#include <string>
#include <Psapi.h>
#include "studio.h"
#include "gles/GLESRenderBase.h"
#include "gles/GLESRenderer.h"
#include "interface/RenderState.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/io/IO.h>

namespace Studio
{
	std::string	EchoEngine::m_projectFile;		// 项目名称
	RenderWindow* EchoEngine::m_renderWindow = NULL;

	EchoEngine::EchoEngine()
		: m_log(NULL)
		, m_currentEditNode(nullptr)
		, m_invisibleNodeForEditor(nullptr)
		, m_gizmosNodeBackGrid(nullptr)
		, m_gizmosNodeGrid2d(nullptr)
	{
	}

	EchoEngine::~EchoEngine()
	{
	}

	EchoEngine* EchoEngine::instance()
	{
		static EchoEngine* inst = new EchoEngine;
		return inst;
	}

	bool EchoEngine::Initialize(HWND hwnd)
	{
		// init engine by configure
		Echo::Engine::Config rootcfg;
		rootcfg.m_projectFile = !m_projectFile.empty() ? m_projectFile.c_str() : "";
		rootcfg.m_isGame = false;
		rootcfg.m_windowHandle = (size_t)hwnd;
		Echo::Engine::instance()->initialize(rootcfg);

		TIME_PROFILE
		(
			// editor node
			m_invisibleNodeForEditor = ECHO_DOWN_CAST<Echo::Node*>(Echo::Class::create("Node"));

			// gizmos node
			m_gizmosNodeBackGrid = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
			m_gizmosNodeBackGrid->setName("Gizmos");
			m_gizmosNodeBackGrid->setParent(m_invisibleNodeForEditor);

			// gizmos node 2d
			m_gizmosNodeGrid2d = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
			m_gizmosNodeGrid2d->setName("Gizmos 2d grid");
			m_gizmosNodeGrid2d->setParent(m_invisibleNodeForEditor);

			// 背景网格
			InitializeBackGrid();
		);

		m_currentEditNode = nullptr;

		return true;
	}

	void EchoEngine::Release()
	{
		delete this;
	}

	void EchoEngine::Render(float elapsedTime, bool isRenderWindowVisible)
	{
		// update back grid
		resizeBackGrid2d();
		resizeBackGrid3d();

		if (m_currentEditNode)
			m_currentEditNode->update( elapsedTime, true);

		if (m_invisibleNodeForEditor)
			m_invisibleNodeForEditor->update(elapsedTime, true);

		Echo::Engine::instance()->tick(elapsedTime);
	}

	void EchoEngine::Resize(int cx, int cy)
	{
		Echo::Engine::instance()->onSize(cx, cy);

		m_renderWindow->getInputController()->onSizeCamera(Echo::Renderer::instance()->getScreenWidth(), Echo::Renderer::instance()->getScreenHeight());
	}

	bool EchoEngine::SetProject(const char* projectFile)
	{
		m_projectFile = projectFile;

		// 初始化渲染窗口
		m_renderWindow = static_cast<RenderWindow*>(AStudio::instance()->getRenderWindow());

		return true;
	}

	void EchoEngine::setCurrentEditNodeSavePath(const Echo::String& savePath) 
	{ 
		m_currentEditNodeSavePath = savePath;
		if (!m_currentEditNodeSavePath.empty())
		{
			AStudio::instance()->getConfigMgr()->setValue("last_edit_node_tree", m_currentEditNodeSavePath.c_str());
		}
	}

	// new
	void EchoEngine::newEditNodeTree()
	{
		setCurrentEditNodeSavePath("");
		if (m_currentEditNode)
		{
			m_currentEditNode->queueFree();
			m_currentEditNode = nullptr;
		}
	}

	// on open node tree
	bool EchoEngine::onOpenNodeTree(const Echo::String& resPath)
	{
		Echo::Node* node = Echo::Node::loadLink(resPath, false);

		setCurrentEditNode(node);
		setCurrentEditNodeSavePath(resPath);

		// recove input controller parameters
		AStudio::instance()->getRenderWindow()->getInputController2D()->onOpenNodeTree(resPath);
		AStudio::instance()->getRenderWindow()->getInputController3D()->onOpenNodeTree(resPath);

		return node ? true : false;
	}

	// save current node tree
	void EchoEngine::saveCurrentEditNodeTree()
	{
		if (m_currentEditNode && !m_currentEditNodeSavePath.empty())
		{
			saveBranchAsScene( m_currentEditNodeSavePath, m_currentEditNode);
		}
	}

	void EchoEngine::saveCurrentEditNodeTreeAs(const Echo::String& savePath)
	{
		saveBranchAsScene(savePath, m_currentEditNode);
	}

	void EchoEngine::saveBranchAsScene(const Echo::String& savePath, Echo::Node* node)
	{
		if (node && !savePath.empty())
		{
			node->save(savePath);
		}
	}

	void EchoEngine::InitializeBackGrid()
	{
		resizeBackGrid2d();
		resizeBackGrid3d();
	}

	void EchoEngine::resizeBackGrid3d()
	{	
		static int xOffsetBefore = 0.f;
		static int zOffsetBefore = 0.f;

		// center pos
		Echo::Vector3 centerPos = Echo::NodeTree::instance()->get3dCamera()->getPosition();
		int xOffset = centerPos.x;
		int yOffset = abs(centerPos.y);
		int zOffset = centerPos.z;

		// calc y alpha scale
		float startGrayFadeDistance = 10.f;
		float endGrayFadeDistance = 35.f;
		float yGrayAlphaScale = 1.f - Echo::Math::Clamp((float)abs(yOffset), startGrayFadeDistance, endGrayFadeDistance) / (endGrayFadeDistance - startGrayFadeDistance);

		float startBlueFadeDistance = 10.f;
		float endBlueFadeDistance = 200.f;
		float yBlueAlphaScale = 1.f - Echo::Math::Clamp((float)abs(yOffset), startBlueFadeDistance, endBlueFadeDistance) / (endBlueFadeDistance - startBlueFadeDistance);
			
		if (xOffset != xOffsetBefore || zOffset != zOffsetBefore)
		{
			m_gizmosNodeBackGrid->clear();
			m_gizmosNodeBackGrid->set2d(false);

			// gray line
			if (yOffset < endGrayFadeDistance)
			{
				int lineNum = (80 + abs(yOffset)) / 10 * 10;
				for (int i = -lineNum; i <= lineNum; i++)
				{
					Echo::Color color = Echo::Color(0.5f, 0.5f, 0.5f, 0.4f * yGrayAlphaScale);

					// xaxis
					int xAxis = xOffset + i;
					if (xAxis % 10 != 0)
						m_gizmosNodeBackGrid->drawLine(Echo::Vector3(xAxis, 0.f, -lineNum + zOffset), Echo::Vector3(xAxis, 0.f, lineNum + zOffset), color);

					int zAxis = zOffset + i;
					if (zAxis % 10 != 0)
						m_gizmosNodeBackGrid->drawLine(Echo::Vector3(-lineNum + xOffset, 0.f, zAxis), Echo::Vector3(lineNum + xOffset, 0.f, zAxis), color);
				}
			}

			// blue line
			int xOffset10 = xOffset / 10;
			int zOffset10 = zOffset / 10;
			int lineNum = (80 + abs(yOffset * 5)) / 10;
			for (int i = -lineNum; i <= lineNum; i++)
			{
				// xaxis
				int xAxis = xOffset10 + i;
				Echo::Color color = Echo::Color(0.8f, 0.5, 0.5f, 0.5f * yBlueAlphaScale);
				m_gizmosNodeBackGrid->drawLine(Echo::Vector3(xAxis * 10.f, 0.f, (-lineNum + zOffset10)*10.f), Echo::Vector3(xAxis * 10.f, 0.f, (lineNum + zOffset10)*10.f), color);

				int zAxis = zOffset10 + i;
				m_gizmosNodeBackGrid->drawLine(Echo::Vector3((-lineNum + xOffset10)*10.f, 0.f, zAxis * 10.f), Echo::Vector3((lineNum + xOffset10)*10.f, 0.f, zAxis*10.f), color);
			}

			xOffsetBefore = xOffset;
			zOffsetBefore = zOffset;
		}
	}

	void EchoEngine::resizeBackGrid2d()
	{
		static Echo::i32 curWindowHalfWidth = -1;
		static Echo::i32 curWindowHalfHeight = -1;
		Echo::i32 windowHalfWidth = Echo::GameSettings::instance()->getDesignWidth() / 2;
		Echo::i32 windowHalfHeight = Echo::GameSettings::instance()->getDesignHeight() / 2;
		if (curWindowHalfWidth != windowHalfWidth || curWindowHalfHeight != windowHalfHeight)
		{
			m_gizmosNodeGrid2d->clear();
			m_gizmosNodeGrid2d->set2d(true);
			m_gizmosNodeGrid2d->drawLine(Echo::Vector3(-windowHalfWidth, -windowHalfHeight, 0.0), Echo::Vector3(windowHalfWidth, -windowHalfHeight, 0.0), Echo::Color::RED);
			m_gizmosNodeGrid2d->drawLine(Echo::Vector3(-windowHalfWidth, -windowHalfHeight, 0.0), Echo::Vector3(-windowHalfWidth, windowHalfHeight, 0.0), Echo::Color::RED);
			m_gizmosNodeGrid2d->drawLine(Echo::Vector3(windowHalfWidth, windowHalfHeight, 0.0), Echo::Vector3(windowHalfWidth, -windowHalfHeight, 0.0), Echo::Color::RED);
			m_gizmosNodeGrid2d->drawLine(Echo::Vector3(windowHalfWidth, windowHalfHeight, 0.0), Echo::Vector3(-windowHalfWidth, windowHalfHeight, 0.0), Echo::Color::RED);

			curWindowHalfWidth = windowHalfWidth;
			curWindowHalfHeight = windowHalfHeight;
		}
	}

	void EchoEngine::setBackGridVisibleOrNot(bool showFlag)
	{
		/*
		if (m_backGrid && m_backGridNode)
		{
			if (!showFlag)
			{
				m_backGrid->attachTo(NULL);
			}
			else
			{
				m_backGrid->attachTo(m_backGridNode);
			}
		}
		*/
	}

	void EchoEngine::previewAudioEvent(const char* audioEvent)
	{
		//Echo::FSAudioManager::instance()->destroyAudioSources(&m_curPlayAudio, 1);

		//bool isAudioEvnet = Echo::StringUtil::StartWith(audioEvent, "event:", true);
		//if (isAudioEvnet)
		//{
		//	Echo::AudioSource::Cinfo cinfo;
		//	cinfo.m_name = audioEvent;
		//	cinfo.m_is3DMode = false;
		//	m_curPlayAudio = Echo::FSAudioManager::instance()->createAudioSource(cinfo);
		//}
	}

	void EchoEngine::stopCurPreviewAudioEvent()
	{
		//Echo::FSAudioManager::instance()->destroyAudioSources(&m_curPlayAudio, 1);
	}

	float EchoEngine::GetMeshRadius()
	{
		return 10.f;
	}

	void EchoEngine::SaveSceneThumbnail(bool setCam)
	{
		//Echo::RenderTarget* defaultBackBuffer = Echo::RenderTargetManager::Instance()->getRenderTargetByID(Echo::RTI_DefaultBackBuffer);
		//if (defaultBackBuffer)
		//{
		//	Echo::String sceneFullPath = EchoResourceManager->getFileLocation(EchoSceneManager->getCurrentScene()->getSceneName() + ".scene");
		//	Echo::String sceneLocation = Echo::PathUtil::GetFileDirPath(sceneFullPath);
		//	Echo::String bmpSavePath = Echo::PathUtil::GetRenameExtFile(sceneFullPath, ".bmp");
		//	if (setCam)
		//		defaultBackBuffer->saveTo((std::string(bmpSavePath.c_str())).c_str());
		//	else
		//		defaultBackBuffer->saveTo((std::string(sceneLocation.c_str()) + "/map.bmp").c_str());
		//}
	}	
}
