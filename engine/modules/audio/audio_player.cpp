#include "audio_player.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"
#include "dr_libs/dr_flac.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
	AudioPlayer::AudioPlayer()
	{
		alGenSources(1, &m_source);
		alGenBuffers(1, &m_buffer);
	}

	AudioPlayer::~AudioPlayer()
	{
		alDeleteSources(1, &m_source);
		alDeleteBuffers(1, &m_buffer);
	}

	void AudioPlayer::bindMethods()
	{
		CLASS_BIND_METHOD(AudioPlayer, is2d,		DEF_METHOD("is2d"));
		CLASS_BIND_METHOD(AudioPlayer, set2d,		DEF_METHOD("set2d"));
        CLASS_BIND_METHOD(AudioPlayer, getAudio,	DEF_METHOD("getAudio"));
        CLASS_BIND_METHOD(AudioPlayer, setAudio,	DEF_METHOD("setAudio"));

		CLASS_REGISTER_PROPERTY(AudioPlayer, "Is2D", Variant::Type::Bool, "is2d", "set2d");
        CLASS_REGISTER_PROPERTY(AudioPlayer, "Audio", Variant::Type::ResourcePath, "getAudio", "setAudio");
	}

	void AudioPlayer::setPitch(float pitch)
	{
		m_pitch = pitch;

		alSourcef(m_source, AL_PITCH, 1);
	}

	void AudioPlayer::setGain(float gain)
	{
		m_gain = gain;

		alSourcef(m_source, AL_GAIN, 1);
	}

	void AudioPlayer::setLoop(bool loop)
	{
		m_isLoop = loop;

		alSourcei( m_source, AL_LOOPING, m_isLoop);
	}

	bool AudioPlayer::isPlaying()
	{
		ALenum state;

		alGetSourcei(m_source, AL_SOURCE_STATE, &state);

		return (state == AL_PLAYING);
	}

	void AudioPlayer::update_self()
	{
		const Vector3& position = getWorldPosition();
		
		// update self position
		updatePosition(position);

		// one shot players
		if (!m_oneShotPlayers.empty())
		{
			for (AudioPlayerArray::iterator it=m_oneShotPlayers.begin(); it!=m_oneShotPlayers.end(); )
			{
				AudioPlayer* player = *it;
				if (player->isPlaying())
				{
					player->updatePosition(position);
					it++;
				}
				else
				{
					m_oneShotPlayers.erase(it++);
				}
			}
		}
	}

	void AudioPlayer::updatePosition(const Vector3& position)
	{
		alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
		alSource3f(m_source, AL_VELOCITY, 0.f, 0.f, 0.f);
	}

	void AudioPlayer::play()
	{    
		//assign the buffer to this source
		alSourcei(m_source, AL_BUFFER, m_buffer);

        // play
        alSourcePlay( m_source);
	}
    
    void AudioPlayer::pause()
    {
        
    }
    
    void AudioPlayer::stop()
    {
        
    }
    
    void AudioPlayer::setAudio(const ResourcePath& res)
    {
        m_audioRes=res;

		loadBuff();

		if (m_isPlay && IsGame)
		{
			play();
		}
    }
    
    bool AudioPlayer::loadBuff()
    {
		MemoryReader memReader(m_audioRes.getPath());
		if (memReader.getSize())
		{
			drmp3 mp3;
			drmp3_config config;
			config.outputChannels = 1;
			config.outputSampleRate = DR_MP3_DEFAULT_SAMPLE_RATE;
			if (drmp3_init_memory(&mp3, memReader.getData<const void*>(), memReader.getSize(), &config))
			{
				drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);

				drmp3_uint32 channels = mp3.channels;
				float* audioBuffer = new float[frameCount];
				i16* audioBuffer16 = new i16[frameCount];
				drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(&mp3, frameCount, audioBuffer);
				if (framesRead > 0)
				{
					ALenum  audioFormat = AL_FORMAT_MONO16;
					ALint   audioBufferLen = framesRead * sizeof(i16);
					ALint   audioFrequency = mp3.sampleRate;

					drwav_f32_to_s16(audioBuffer16, audioBuffer, framesRead);
					alBufferData(m_buffer, audioFormat, audioBuffer16, audioBufferLen, audioFrequency);
				}

				delete[] audioBuffer;
				delete[] audioBuffer16;

				return true;
			}
		}
        
        return false;
    }

	void AudioPlayer::playOneShot(const char* res)
	{
		AudioPlayer* newPlayer = EchoNew(AudioPlayer);
		if(newPlayer)
		{
			newPlayer->setAudio(ResourcePath(res));
			newPlayer->updatePosition(getWorldPosition());
			newPlayer->setLoop(false);
			newPlayer->set2d(is2d());
			newPlayer->play();

			m_oneShotPlayers.push_back(newPlayer);
		}
	}
}
