#include "EngineConsole.h"
#include "Engine/core/Scene/Scene_Manager.h"
#include "engine/core/render/render/Viewport.h"
#include "engine/core/render/render/Renderer.h"

namespace Echo
{
	// 执行指令
	void EngineConsole::exec( const String& cmd, String& output)
	{
		StringArray argv = StringUtil::Split( cmd, " ");
		parse( argv, output);
	}

	// 解析
	void EngineConsole::parse( const StringArray& argv, String& output)
	{
		// 渲染队列
		if( argv[0] == "renderqueue")
		{
			parseRenderQueue( argv, output);
		}
		else if( argv[0] == "get")
		{
			parseGet( argv, output);
		}
		else
		{
			output += StringUtil::Format( "unknown command [%s] \n", argv[0].c_str());
		}
	}

	//渲染队列相关处理
	void EngineConsole::parseRenderQueue( const StringArray& argv, String& output)
	{
		std::string param0 = argv.size()>1 ? argv[1].c_str() : "";
		if( param0 == "info")
		{
			// 列出所有渲染队列
			size_t rqCount = SceneManager::instance()->getRenderQueueCount();
			output += StringUtil::Format( "\n%-8s%-40s%-10s\n", "index", "name", "state");

			for( size_t i=0; i<rqCount; i++)
			{
				RenderQueue* pQueue = SceneManager::instance()->getRenderQueueByIndex(i);
				if( pQueue)
				{
					output += StringUtil::Format( "%-8d%-40s%-10s\n", i, pQueue->getName().c_str(), pQueue->isEnable() ? "enable" : "disable");
				}
			}
		}
		else if( param0 == "enable")
		{
			// 开启渲染
			std::string param1 = argv.size()>2 ? argv[2].c_str() : "";
			if( param1=="all")
			{
				// 启用所有
				size_t rqCount = SceneManager::instance()->getRenderQueueCount();
				for( size_t i=0; i<rqCount; i++)
				{
					RenderQueue* pQueue = SceneManager::instance()->getRenderQueueByIndex(i);
					if( pQueue)
						pQueue->enableRender();
				}
			}
			else
			{
				// 启用指定队列
				RenderQueue* pQueue = SceneManager::instance()->getRenderQueue(param1.c_str());
				if( pQueue)
					pQueue->enableRender();
				else
					output += StringUtil::Format( "renderqueue[%s] not exist \n", param1.c_str());
			}

		}
		else if( param0 == "disable")
		{
			std::string param1 = argv.size()>2 ? argv[2].c_str() : "";
			if( param1=="all")
			{
				// 禁用所有
				size_t rqCount = SceneManager::instance()->getRenderQueueCount();
				for( size_t i=0; i<rqCount; i++)
				{
					RenderQueue* pQueue = SceneManager::instance()->getRenderQueueByIndex(i);
					if( pQueue)
						pQueue->disableRender();

				}
			}
			else
			{
				// 禁用指定队列
				RenderQueue* pQueue = SceneManager::instance()->getRenderQueue(param1.c_str());
				if( pQueue)
					pQueue->disableRender();
				else
					output += StringUtil::Format( "renderqueue[%s] not exist \n", param1.c_str());
			}
		}
		else
		{
			output += StringUtil::Format( "unknown param [%s] \n", param0.c_str());
		}
	}

	// 获取变量
	void EngineConsole::parseGet( const StringArray& argv, String& output)
	{
		std::string param0 = argv.size()>1 ? argv[1].c_str() : "";
		if( param0 == "rendererinfo")
		{
			Viewport viewPort;
			Renderer::instance()->getViewportReal( viewPort);

			output += StringUtil::Format("\n%-15s[%4d,%4d,%4d,%4d]", "ViewPort:", viewPort.getLeft(), viewPort.getTop(), Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight());
			output += StringUtil::Format( "\n%-15s[%4d,%4d,%4d,%4d]\n", "ViewPortReal:", viewPort.getLeft(), viewPort.getTop(), viewPort.getWidth(), viewPort.getHeight());
		}
		else
		{
			output += StringUtil::Format( "unknown param [%s] \n", param0.c_str());
		}
	}
}