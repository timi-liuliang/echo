#pragma once

#include "engine/core/scene/node.h"
#include "audio_base.h"

namespace Echo
{
	class AudioPlayer : public Node 
	{
		ECHO_CLASS(AudioPlayer, Node)

	public:
		typedef vector<AudioPlayer*>::type AudioPlayerArray;

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
        void setAudio(const ResourcePath& res);
        const ResourcePath& getAudio() const { return m_audioRes; }

		// special operate
		void playOneShot(const char* res);

	protected:
        // start
        virtual void start() override;
        
		// update
		virtual void update_self() override;

		// update position
		void updatePosition(const Vector3& position);
        
    private:
        // load audio data from file
        bool loadBuff();

	private:
		ALuint				m_source = -1;
		ALuint				m_buffer;
		float				m_pitch;
		float				m_gain;
		bool				m_isLoop = false;
		bool				m_isPlayOnAwake = true;
		bool				m_is2D = true;
        ResourcePath		m_audioRes = ResourcePath("", ".mp3|.flac|.wav|.audio");
		AudioPlayerArray	m_oneShotPlayers;
	};
}
