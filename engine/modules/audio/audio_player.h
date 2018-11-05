#pragma once

#include "engine/core/scene/node.h"
#include <al.h>
#include <alc.h>

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

		// play
		void play();

	protected:
		// update
		virtual void update_self();

	private:
		ALuint		m_source;
		ALuint		m_buffer;
		float		m_pitch;
		float		m_gain;
		bool		m_isLoop;
	};
}