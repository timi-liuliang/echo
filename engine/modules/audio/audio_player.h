#pragma once

#include "engine/core/scene/node.h"
#include "audio_base.h"

namespace Echo
{
	class AudioPlayer : public Node 
	{
		ECHO_CLASS(AudioPlayer, Node)

	public:
		AudioPlayer();
		virtual ~AudioPlayer();

		// pitch
		float getPitch() const { return m_pitch; }
		void setPitch(float pitch);

		// gain
		float getGain() const { return m_gain; }
		void setGain(float gain);

		// loop
		bool isLoop() const { return m_isLoop; }
		void setLoop(bool loop);

		// operates
		void play();
        void pause();
        void stop();
        
        // audio file
        void setAudio(const ResourcePath& res);
        const ResourcePath& getAudio() const { return m_audioRes; }

	protected:
		// update
		virtual void update_self() override;
        
    private:
        // load audio data from file
        bool loadBuff();

	private:
		ALuint		    m_source;
		ALuint		    m_buffer;
		float		    m_pitch;
		float		    m_gain;
		bool		    m_isLoop;
		bool			m_isPlay = true;
		bool			m_is2D = true;
        ResourcePath    m_audioRes = ResourcePath("", ".mp3|.flac|.wav");
	};
}
