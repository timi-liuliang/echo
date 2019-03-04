#include "audio_player.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"
#include "dr_libs/dr_flac.h"
#include "dr_libs/dr_wav.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
	AudioPlayer::AudioPlayer()
	{
	}

	AudioPlayer::~AudioPlayer()
	{
		alDeleteSources(1, &m_source);
		alDeleteBuffers(1, &m_buffer);
	}

	void AudioPlayer::bindMethods()
	{
        CLASS_BIND_METHOD(AudioPlayer, getAudio, DEF_METHOD("getAudio"));
        CLASS_BIND_METHOD(AudioPlayer, setAudio, DEF_METHOD("setAudio"));

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

	void AudioPlayer::update_self()
	{
		const Vector3& position = getWorldPosition();

		alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
		alSource3f(m_source, AL_VELOCITY, 0.f, 0.f, 0.f);
	}

	void AudioPlayer::play()
	{
		alGenSources(1, &m_source);
		alGenBuffers(1, &m_buffer);
        
        //put the data into our sampleset buffer
        loadBuff();
        
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
        
        MemoryReader memReader( m_audioRes.getPath());
        if(memReader.getSize())
        {
            alGenSources(1, &m_source);
            alGenBuffers(1, &m_buffer);
            
            drmp3 mp3;
            drmp3_config config;
            config.outputChannels = 1;
            config.outputSampleRate = DR_MP3_DEFAULT_SAMPLE_RATE;
            if(drmp3_init_memory( &mp3, memReader.getData<const void*>(), memReader.getSize(), &config))
            {
                drmp3_uint32 channels = mp3.channels;
                ALchar* audioBuffer[4*10000];
                drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(&mp3, 10000, (float*)audioBuffer);
                if(framesRead>0)
                {
                    ALenum  audioFormat = AL_FORMAT_MONO16;
                    ALint   audioBufferLen = framesRead * sizeof(float);
                    ALint   audioFrequency = mp3.sampleRate;
                    
                    alBufferData(m_buffer, audioFormat, audioBuffer, audioBufferLen, audioFrequency);
                    
                    //framesRead = drmp3_read_pcm_frames_f32(&mp3, 100000, (float*)audioBuffer);
                }
                
                //assign the buffer to this source
                alSourcei(m_source, AL_BUFFER, m_buffer);
                
                // play
                alSourcePlay(m_source);
            }
        }
    }
    
    bool AudioPlayer::loadBuff()
    {
        ALenum  audioFormat = 0;
        ALchar* audioBuffer = nullptr;
        ALint   audioBufferLen = 0;
        ALint   audioFrequency = 0;
        
        alBufferData(m_buffer, audioFormat, audioBuffer, audioBufferLen, audioFrequency);
        
        return true;
    }
    
    bool AudioPlayer::loadWav()
    {
         return false;
    }
    
    bool AudioPlayer::loadFlac()
    {
        return false;
    }
    
    bool AudioPlayer::loadMp3()
    {
        return false;
    }
}
