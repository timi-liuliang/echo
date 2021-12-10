#include "video_player.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	VideoPlayer::VideoPlayer()
	{
	}

	VideoPlayer::~VideoPlayer()
	{
	}

	void VideoPlayer::bindMethods()
	{
		CLASS_BIND_METHOD(VideoPlayer, is2d);
		CLASS_BIND_METHOD(VideoPlayer, set2d);
        CLASS_BIND_METHOD(VideoPlayer, isLoop);
        CLASS_BIND_METHOD(VideoPlayer, setLoop);
        CLASS_BIND_METHOD(VideoPlayer, isPlayOnAwake);
        CLASS_BIND_METHOD(VideoPlayer, setPlayOnAwake);
        CLASS_BIND_METHOD(VideoPlayer, getVideo);
        CLASS_BIND_METHOD(VideoPlayer, setVideo);

		CLASS_REGISTER_PROPERTY(VideoPlayer, "Is2D", Variant::Type::Bool, is2d, set2d);
        CLASS_REGISTER_PROPERTY(VideoPlayer, "Loop", Variant::Type::Bool, isLoop, setLoop);
        CLASS_REGISTER_PROPERTY(VideoPlayer, "PlayOnAwake", Variant::Type::Bool, isPlayOnAwake, setPlayOnAwake);
        CLASS_REGISTER_PROPERTY(VideoPlayer, "Volume", Variant::Type::Real, getVolume, setVolume);
        CLASS_REGISTER_PROPERTY(VideoPlayer, "Video", Variant::Type::ResourcePath, getVideo, setVideo);
	}

	void VideoPlayer::setLoop(bool loop)
	{
		m_isLoop = loop;
	}
    
    void VideoPlayer::set2d(bool is2d)
    {     
        m_is2D = is2d;
    }

	bool VideoPlayer::isPlaying()
	{
		return false;
	}
    
    void VideoPlayer::start()
    {
        if (m_isPlayOnAwake && IsGame)
        {
            play();
        }
    }

	void VideoPlayer::updateInternal(float elapsedTime)
	{
		const Vector3& position = getWorldPosition();
		
		// update self position
		updatePosition(position);
	}

	void VideoPlayer::updatePosition(const Vector3& position)
	{

	}

	void VideoPlayer::play()
	{
		if (m_isCustomRender)
		{
			m_videoRender = nullptr;
		}

		m_jplayer = EchoNew(cmpeg::player(m_videoRender, m_videoRes.getPath().c_str()));
		m_jplayer->play();
	}
    
    void VideoPlayer::pause()
    {

    }
    
    void VideoPlayer::stop()
    {

    }
    
    void VideoPlayer::setVideo(const ResourcePath& res)
    {
        m_videoRes=res;
    }
}
