#include "audio_player.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"
#include "dr_libs/dr_flac.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	AudioPlayer::AudioPlayer()
	{
		alGenSources(1, &m_source);
		alGenBuffers(1, &m_buffer);
	}

	AudioPlayer::~AudioPlayer()
	{
        if(m_source!=-1)
        {
            stop();
            
            alDeleteSources(1, &m_source);
            alDeleteBuffers(1, &m_buffer);
        }

		EchoSafeDeleteContainer(m_oneShotPlayers, AudioPlayer);
	}

	void AudioPlayer::bindMethods()
	{
        CLASS_BIND_METHOD(AudioPlayer, playOneShot,         DEF_METHOD("playOneShot"));
		CLASS_BIND_METHOD(AudioPlayer, is2d,		        DEF_METHOD("is2d"));
		CLASS_BIND_METHOD(AudioPlayer, set2d,		        DEF_METHOD("set2d"));
        CLASS_BIND_METHOD(AudioPlayer, isLoop,              DEF_METHOD("isLoop"));
        CLASS_BIND_METHOD(AudioPlayer, setLoop,             DEF_METHOD("setLoop"));
        CLASS_BIND_METHOD(AudioPlayer, getVolume,           DEF_METHOD("getVolume"));
        CLASS_BIND_METHOD(AudioPlayer, setVolume,           DEF_METHOD("setVolume"));
        CLASS_BIND_METHOD(AudioPlayer, isPlayOnAwake,       DEF_METHOD("isPlayOnAwake"));
        CLASS_BIND_METHOD(AudioPlayer, setPlayOnAwake,      DEF_METHOD("setPlayOnAwake"));
        CLASS_BIND_METHOD(AudioPlayer, getAudio,	        DEF_METHOD("getAudio"));
        CLASS_BIND_METHOD(AudioPlayer, setAudio,	        DEF_METHOD("setAudio"));

		CLASS_REGISTER_PROPERTY(AudioPlayer, "Is2D", Variant::Type::Bool, "is2d", "set2d");
        CLASS_REGISTER_PROPERTY(AudioPlayer, "Loop", Variant::Type::Bool, "isLoop", "setLoop");
        CLASS_REGISTER_PROPERTY(AudioPlayer, "PlayOnAwake", Variant::Type::Bool, "isPlayOnAwake", "setPlayOnAwake");
        CLASS_REGISTER_PROPERTY(AudioPlayer, "Volume", Variant::Type::Real, "getVolume", "setVolume");
        CLASS_REGISTER_PROPERTY(AudioPlayer, "Audio", Variant::Type::ResourcePath, "getAudio", "setAudio");
	}

	void AudioPlayer::setPitch(float pitch)
	{
		m_pitch = pitch;

		alSourcef(m_source, AL_PITCH, 1);
	}

	void AudioPlayer::setVolume(float gain)
	{
		m_gain = gain;

		alSourcef(m_source, AL_GAIN, m_gain);
	}

	void AudioPlayer::setLoop(bool loop)
	{
		m_isLoop = loop;

		alSourcei( m_source, AL_LOOPING, m_isLoop);
	}
    
    void AudioPlayer::set2d(bool is2d)
    {
        if(is2d)
        {
            alSourcei( m_source, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        }
        else
        {
            alSourcei( m_source, AL_SOURCE_RELATIVE, AL_FALSE);
        }
        
        m_is2D = is2d;
    }

	bool AudioPlayer::isPlaying()
	{
		ALenum state;

		alGetSourcei(m_source, AL_SOURCE_STATE, &state);

		return (state == AL_PLAYING);
	}
    
    void AudioPlayer::start()
    {
        if (m_isPlayOnAwake && IsGame)
        {
            play();
        }
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
					it = m_oneShotPlayers.erase(it);
					EchoSafeDelete(player, AudioPlayer);
				}
			}
		}
	}

	void AudioPlayer::updatePosition(const Vector3& position)
	{
        if(!m_is2D)
        {
            alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
            alSource3f(m_source, AL_VELOCITY, 0.f, 0.f, 0.f);
        }
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
        alSourcePause( m_source);
    }
    
    void AudioPlayer::stop()
    {
        alSourceStop( m_source);
    }
    
    void AudioPlayer::setAudio(const ResourcePath& res)
    {
        m_audioRes=res;
		if (!m_audioRes.isEmpty())
		{
			loadBuff();
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

				//drmp3_uint32 channels = mp3.channels;
				float* audioBuffer = new float[frameCount];
				i16* audioBuffer16 = new i16[frameCount];
				drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(&mp3, frameCount, audioBuffer);
				if (framesRead > 0)
				{
					ALenum  audioFormat = AL_FORMAT_MONO16;
					ALint   audioBufferLen = static_cast<ALint>(framesRead * sizeof(i16));
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

	void AudioPlayer::playOneShot(const char* res, float volumeScale)
	{
		AudioPlayer* newPlayer = EchoNew(AudioPlayer);
		if(newPlayer)
		{
			newPlayer->setAudio(ResourcePath(res));
			newPlayer->updatePosition(getWorldPosition());
			newPlayer->setLoop(false);
			newPlayer->set2d(is2d());
			newPlayer->setVolume(volumeScale);
			newPlayer->play();

			m_oneShotPlayers.emplace_back(newPlayer);
		}
	}
}
