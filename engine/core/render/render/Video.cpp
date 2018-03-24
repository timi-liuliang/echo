#if !defined(NO_THEORA_PLAYER)
#include "Video.h"
#include "Render/Renderable.h"
#include "Engine/Object/Root.h"
#include <Engine/audio/OpenAL/OpenAL_AudioInterface.h>
#include "Foundation/FileIO/ResourceGroupManager.h"
#include <theoraplayer/TheoraPlayer.h>
#include <theoraplayer/TheoraDataSource.h>

#include "Memory/MemManager.h"


namespace Echo
{
	// 构造函数
	VideoPlay::VideoPlay()
	{
		m_pTex = NULL;

		m_elapsed = 0.0;

		m_width = 256;
		m_height = 256;

		m_bStarted = false;
		m_bSilent = false;

		m_mgr = NULL;
		m_clip = NULL;
		m_iface_factory = NULL;
	}

	VideoPlay::~VideoPlay()
	{
		destroy();
	}

	i32 nextPow2(const i32 x)
	{
		i32 y;
		for (y = 1; y < x; y *= 2);

		return y;
	}

	// 初始化
	bool VideoPlay::parseVideoFile(const String& name, ui32& width, ui32& height)
	{
		m_fileName = name;
		//destroy();

		if (!m_mgr)
		{
			m_mgr = EchoNew(TheoraVideoManager);
		}

		if (!m_iface_factory)
		{
			m_iface_factory = new OpenAL_AudioInterfaceFactory();
			m_mgr->setAudioInterfaceFactory(m_iface_factory);
		}
		
		if (m_clip)
		{
			m_mgr->destroyVideoClip(m_clip);
			m_clip = NULL;
		}
		TheoraOutputMode outputMode = TH_RGB;
		String filename;
#if defined(ECHO_PLATFORM_WINDOWS)
		filename = IO::instance()->getFileLocation(m_fileName);
#endif
		if (filename == "")
			filename = m_fileName;
		m_clip = m_mgr->createVideoClip(filename.c_str(), outputMode, 16);
		m_width = m_clip->getWidth();
		m_height = m_clip->getHeight();
		width = m_width;
		height = m_height;

		m_clip->setAutoRestart(true);

		//m_pTex = EchoNew(Echo::TextureRes(m_fileName, true));
		if (NULL != m_pTex)
		{
			assert(false);
			return false;
		}
		m_pTex = ECHO_DOWN_CAST<TextureRes*>(EchoTextureResManager->createResource(m_fileName, true));

		m_mgr->update(1.0f / 1000.0f);
		TheoraVideoFrame* f = m_clip->getNextFrame();

		i32 w = nextPow2(width);
		i32 h = nextPow2(height);
		void* b = EchoMalloc(w * h * 3);//new unsigned char[w * h * 3];
		memset(b, 0, w * h * 3);
		Echo::Buffer buf(w * h * 3, b, true);
		bool r = m_pTex->reCreate2D(Echo::PF_RGB8_UNORM, Echo::Texture::TT_2D, w, h, 0, buf);

		if (f)
		{
			Rect rect(Echo::Real(0), Echo::Real(0), Echo::Real(m_width), Echo::Real(m_height));
			m_pTex->updateSubTex2D(0, rect, (void*)f->getBuffer(), m_width*m_height * 3);
		}

		return true;
	}

	// 更新
	void VideoPlay::tick(Echo::ui32 elapsedTime)
	{
		if (m_bStarted)
		{
			int frameFinished = 0;

			if (!m_clip)
				return;

			float diff = elapsedTime / 1000.0f;
			float frameDuration = static_cast<float>(1.0 / m_clip->getFPS());
			if (diff > frameDuration*2)
			{
				diff = frameDuration*2;
			}

			m_mgr->update(diff);
			m_elapsed += diff;
			TheoraVideoFrame* f = m_clip->getNextFrame();
			if (f)
			{
				Rect rect(Echo::Real(0), Echo::Real(0), Echo::Real(m_width), Echo::Real(m_height));
				m_pTex->updateSubTex2D(0, rect, (void*)f->getBuffer(), m_width*m_height*3);
			}

			// 结束
			if (m_elapsed + frameDuration >= m_clip->getDuration())
			{
				m_elapsed = 0.0f;
				stop();

				// 回掉通知结束
				if (m_videoEndCallback)
					m_videoEndCallback();
			}
		}
	}

	// 销毁
	void VideoPlay::destroy()
	{
		//EchoSafeDelete(m_pTex, TextureRes);
		if (m_mgr)
		{
			if (m_clip)
				m_mgr->destroyVideoClip(m_clip);
			if (m_iface_factory)
			{
				delete m_iface_factory;
				m_iface_factory = nullptr;
			}
			m_clip = NULL;
			EchoSafeDelete(m_mgr, TheoraVideoManager);
			m_elapsed = 0.0f;
		}
	}

	void VideoPlay::play()
	{
		//  use this if you want to preload the file into ram and stream from there
		//	clip=mgr->createVideoClip(new TheoraMemoryFileDataSource("../media/short" + resourceExtension),TH_RGB);
		//m_clip->setAutoRestart(1);
		if(m_clip)
			m_clip->play();
		m_bStarted = true;
		m_elapsed = 0.0f;
	}

	void VideoPlay::pause()
	{
		if(m_clip)
			if (!m_clip->isPaused())
				m_clip->pause();

		m_bStarted = false;
	}

	void VideoPlay::stop()
	{
		if (m_clip)
		{
			m_clip->stop();
			m_mgr->destroyVideoClip(m_clip);
			m_clip = NULL;
		}
			
		m_bStarted = false;
		m_elapsed = 0.0f;
		//destroy();
	}

	void VideoPlay::restart()
	{
		if(m_clip)
		{
			m_clip->restart();
			m_clip->play();
		}
		
		m_bStarted = true;
		m_elapsed = 0.0f;
		
	}

	void VideoPlay::setSilent(bool silent)
	{
		m_bSilent = silent;
		if (m_clip && m_iface_factory && m_iface_factory->m_interface)
			m_iface_factory->m_interface->setSilent(silent);
	}

	void VideoPlay::releaseTexture()
	{
		if (NULL == m_pTex)
			return;
		EchoTextureResManager->releaseResource(m_pTex);
		m_pTex = NULL;
	}

}
#endif  // NO_THEORA_PLAYER
