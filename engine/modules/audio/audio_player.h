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
        void setAudio(const ResourcePath& res) {m_audioRes=res;}
        const ResourcePath& getAudio() const { return m_audioRes; }

	protected:
		// update
		virtual void update_self();
        
    private:
        // load audio data from file
        bool loadBuff();
        bool loadWav();
        bool loadFlac();
        bool loadMp3();

	private:
		ALuint		    m_source;
		ALuint		    m_buffer;
		float		    m_pitch;
		float		    m_gain;
		bool		    m_isLoop;
        ResourcePath    m_audioRes;
	};
}
