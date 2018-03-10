#ifndef _OpenAL_AudioInterface_h
#define _OpenAL_AudioInterface_h

#include <theoraplayer/TheoraAudioInterface.h>
#include <theoraplayer/TheoraVideoClip.h>
#include <theoraplayer/TheoraTimer.h>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <queue>
	
class OpenAL_AudioInterface : public TheoraAudioInterface, TheoraTimer
{
	int mSourceNumChannels;
	int mMaxBuffSize;
	int mBuffSize;
	short *mTempBuffer;
	float mCurrentTimer;

	struct OpenAL_Buffer
	{
		ALuint id;
		int nSamples;
	};
	std::queue<OpenAL_Buffer> mBufferQueue;

	ALuint mSource;
	int mNumProcessedSamples,mNumPlayedSamples;
public:
	OpenAL_AudioInterface(TheoraVideoClip* owner,int nChannels,int freq);
	~OpenAL_AudioInterface();
	void insertData(float* data,int nSamples);

	//! queued audio buffers, expressed in seconds
	float getQueuedAudioSize();

	void update(float time_increase);

	void pause();
	void play();
	void seek(float time);

	void setSilent(bool silent);
};



class OpenAL_AudioInterfaceFactory : public TheoraAudioInterfaceFactory
{
public:
	OpenAL_AudioInterfaceFactory();
	~OpenAL_AudioInterfaceFactory();
	OpenAL_AudioInterface* createInstance(TheoraVideoClip* owner,int nChannels,int freq);
	
	OpenAL_AudioInterface* m_interface;
	
};

#endif
