#pragma once

#include "engine/core/scene/node.h"
#include "video_base.h"
#include <thirdparty/jplayer/player.h>

namespace Echo
{
	class VideoPlayer : public Node 
	{
		ECHO_CLASS(VideoPlayer, Node)

	public:
		typedef vector<VideoPlayer*>::type VideoPlayerArray;

	public:
		VideoPlayer();
		virtual ~VideoPlayer();

		// loop
		bool isLoop() const { return m_isLoop; }
		void setLoop(bool loop);

		// 2d
		bool is2d() const { return m_is2D; }
        void set2d(bool is2d);
        
        // is play on awake
        bool isPlayOnAwake() const { return m_isPlayOnAwake; }
        void setPlayOnAwake(bool isPlayOnAwake) { m_isPlayOnAwake = isPlayOnAwake;}

		// is playing
		bool isPlaying();

		// operates
		void play();
        void pause();
        void stop();
        
        // audio file
        void setVideo(const ResourcePath& res);
        const ResourcePath& getVideo() const { return m_videoRes; }

	protected:
        // start
        virtual void start() override;
        
		// update
		virtual void updateInternal(float elapsedTime) override;

		// update position
		void updatePosition(const Vector3& position);

	private:
		bool				m_isLoop = false;
		bool				m_isPlayOnAwake = true;
		bool				m_is2D = true;
        ResourcePath		m_videoRes = ResourcePath("", ".mp4|.video");
		cmpeg::player*		m_jplayer = nullptr;
		bool				m_isCustomRender = false;
		cmpeg::render_base* m_videoRender = nullptr;
	};
}
