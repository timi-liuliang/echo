//#if !defined(NO_THEORA_PLAYER)
//#include <stdio.h>
//#include "OpenAL_AudioInterface.h"
//#include "Util/LogManager.h"
//
//static ALCdevice* device = nullptr;
//static ALCcontext* context = nullptr;
//
//static ALenum err = AL_NO_ERROR;
//
//static void DebugALError(const char* filename, int lineNum)
//{
//	err = alGetError();
//	switch (err)
//	{
//	case AL_INVALID_NAME:
//		EchoLogError("File Line Number[%s:%d] invalid name.", filename, lineNum);
//		break;
//	case AL_ILLEGAL_ENUM:
//		EchoLogError("File Line Number[%s:%d] illegal enum.", filename, lineNum);
//		break;
//	case AL_INVALID_VALUE:
//		EchoLogError("File Line Number[%s:%d] invalid value.", filename, lineNum);
//		break;
//	case AL_ILLEGAL_COMMAND:
//		EchoLogError("File Line Number[%s:%d] illegal command.", filename, lineNum);
//		break;
//	case AL_OUT_OF_MEMORY:
//		EchoLogError("File Line Number[%s:%d] out of memory.", filename, lineNum);
//		break;
//	default:
//		break;
//	}
//}
//
//#define OALESDebug(Func) if (err == AL_NO_ERROR) \
//{ \
//	Func; \
//	DebugALError(__FILE__, __LINE__); \
//} \
//
//
//short float2short(float f)
//{
//	if      (f >  1) f= 1;
//	else if (f < -1) f=-1;
//	return (short) (f*32767);
//}
//
//OpenAL_AudioInterface::OpenAL_AudioInterface(TheoraVideoClip* owner,int nChannels,int freq) :
//	TheoraAudioInterface(owner,nChannels,freq), TheoraTimer()
//{
//	mSourceNumChannels = mNumChannels;
//	if (mNumChannels > 2)
//	{
//		// ignore audio with more than 2 channels, use only the stereo channels
//		mNumChannels = 2;
//	}
//	mMaxBuffSize = freq * mNumChannels * 2;
//	mBuffSize = 0;
//	mNumProcessedSamples = 0;
//	mCurrentTimer = 0;
//
//	mTempBuffer = new short[mMaxBuffSize];
//	OALESDebug(alGenSources(1, &mSource));
//	owner->setTimer(this);
//	mNumPlayedSamples = 0;
//}
//
//OpenAL_AudioInterface::~OpenAL_AudioInterface()
//{
//	if (mTempBuffer) delete [] mTempBuffer;
//	
//	if (mSource)
//	{
//		OALESDebug(alSourceStop(mSource));
//		OALESDebug(alSourcei(mSource, AL_BUFFER, NULL));
//		OALESDebug(alDeleteSources(1, &mSource));
//	}
//	while (mBufferQueue.size() > 0)
//	{
//		OALESDebug(alDeleteBuffers(1, &mBufferQueue.front().id));
//		mBufferQueue.pop();
//	}
//}
//
//float OpenAL_AudioInterface::getQueuedAudioSize()
//{
//	return ((float) (mNumProcessedSamples - mNumPlayedSamples)) / mFreq;
//}
//
//void OpenAL_AudioInterface::insertData(float* data, int nSamples)
//{
//	float* tempData = NULL;
//	if (mSourceNumChannels > 2)
//	{
//		tempData = new float[nSamples * 2 / mSourceNumChannels + 16]; // 16 padding just in case
//		int i, n;
//		for (n = 0, i = 0; i < nSamples; i += mSourceNumChannels, n += 2)
//		{
//			tempData[n] = data[i];
//			tempData[n + 1] = data[i + 1];
//		}
//		data = tempData;
//		nSamples = n;
//	}
//	//printf("got %d bytes, %d buffers queued\n",nSamples,(int)mBufferQueue.size());
//	for (int i = 0; i < nSamples; i++)
//	{
//		if (mBuffSize < mMaxBuffSize)
//		{
//			mTempBuffer[mBuffSize++]=float2short(data[i]);
//		}
//		if (mBuffSize == mFreq * mNumChannels / 10)
//		{
//			OpenAL_Buffer buff;
//			memset(&buff, 0, sizeof(OpenAL_Buffer));
//			OALESDebug(alGenBuffers(1, &buff.id));
//
//			ALuint format = (mNumChannels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
//			OALESDebug(alBufferData(buff.id, format, mTempBuffer, mBuffSize * 2, mFreq));
//			OALESDebug(alSourceQueueBuffers(mSource, 1, &buff.id));
//			buff.nSamples=mBuffSize/mNumChannels;
//			mNumProcessedSamples+=mBuffSize/mNumChannels;
//			mBufferQueue.push(buff);
//
//			mBuffSize=0;
//
//			int state = AL_SOURCE_STATE;
//			OALESDebug(alGetSourcei(mSource, AL_SOURCE_STATE, &state));
//			if (state != AL_PLAYING)
//			{
//				//alSourcef(mSource,AL_PITCH,0.5); // debug
//				//alSourcef(mSource,AL_SAMPLE_OFFSET,(float) mNumProcessedSamples-mFreq/4);
//				OALESDebug(alSourcePlay(mSource));
//			}
//
//		}
//	}
//	if (tempData)
//	{
//		delete [] tempData;
//	}
//}
//
//void OpenAL_AudioInterface::update(float time_increase)
//{
//	int i = 0;
//	int nProcessed = 0;
//	OpenAL_Buffer buff;
//
//	// process played buffers
//
//	OALESDebug(alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &nProcessed));
//	
//	for (i=0;i<nProcessed;i++)
//	{
//		buff=mBufferQueue.front();
//		mBufferQueue.pop();
//		mNumPlayedSamples+=buff.nSamples;
//		OALESDebug(alSourceUnqueueBuffers(mSource, 1, &buff.id));
//		OALESDebug(alDeleteBuffers(1, &buff.id));
//	}
//	if (nProcessed != 0)
//	{
//		// update offset
//		OALESDebug(alGetSourcef(mSource, AL_SEC_OFFSET, &mCurrentTimer));
//	}
//
//	// control playback and return time position
//	//alGetSourcei(mSource,AL_SOURCE_STATE,&state);
//	//if (state == AL_PLAYING)
//		mCurrentTimer += time_increase;
//
//	mTime = mCurrentTimer + (float) mNumPlayedSamples/mFreq;
//
//	float duration=mClip->getDuration();
//	if (mTime > duration) mTime=duration;
//}
//
//void OpenAL_AudioInterface::pause()
//{
//	OALESDebug(alSourcePause(mSource));
//	TheoraTimer::pause();
//}
//
//void OpenAL_AudioInterface::play()
//{
//	OALESDebug(alSourcePlay(mSource));
//	TheoraTimer::play();
//}
//
//void OpenAL_AudioInterface::seek(float time)
//{
//	OpenAL_Buffer buff;
//
//	OALESDebug(alSourceStop(mSource));
//	while (!mBufferQueue.empty())
//	{
//		buff=mBufferQueue.front();
//		mBufferQueue.pop();
//		OALESDebug(alSourceUnqueueBuffers(mSource, 1, &buff.id));
//		OALESDebug(alDeleteBuffers(1, &buff.id));
//	}
////		int nProcessed;
////		alGetSourcei(mSource,AL_BUFFERS_PROCESSED,&nProcessed);
////		if (nProcessed != 0)
////			nProcessed=nProcessed;
//	mBuffSize=0;
//
//	mCurrentTimer = 0;
//	mNumPlayedSamples=mNumProcessedSamples=(int) (time*mFreq);
//	mTime = time;
//}
//
//void OpenAL_AudioInterface::setSilent(bool silent)
//{
//	float volume = 1.0f;
//	if (silent)
//		volume = 0.0f;
//	
//	OALESDebug(alSourcef(mSource, AL_GAIN, volume));
//
//}
//
//OpenAL_AudioInterfaceFactory::OpenAL_AudioInterfaceFactory():
//	m_interface(nullptr)
//{
//	// openal init is here used only to simplify samples for this plugin
//	// if you want to use this interface in your own program, you'll
//	// probably want to remove the openal init/destory lines
//	device = alcOpenDevice(nullptr);
//	if (alcGetError(device) != ALC_NO_ERROR)
//	{
//		return;
//	}
//	context = alcCreateContext(device, NULL);
//	if (alcGetError(device) != ALC_NO_ERROR)
//	{
//		OALESDebug(alcCloseDevice(device));
//		device = nullptr;
//		return;
//	}
//	OALESDebug(alcMakeContextCurrent(context));
//	if (alcGetError(device) != ALC_NO_ERROR)
//	{
//		OALESDebug(alcDestroyContext(context));
//		context = NULL;
//		OALESDebug(alcCloseDevice(device));
//		device = NULL;
//	}
//}
//
//OpenAL_AudioInterfaceFactory::~OpenAL_AudioInterfaceFactory()
//{
//	if (device)
//	{
//		OALESDebug(alcMakeContextCurrent(NULL));
//		OALESDebug(alcDestroyContext(context));
//		OALESDebug(alcCloseDevice(device));
//		m_interface = nullptr;
//	}
//}
//
//OpenAL_AudioInterface* OpenAL_AudioInterfaceFactory::createInstance(TheoraVideoClip* owner,int nChannels,int freq)
//{
//	m_interface = new OpenAL_AudioInterface(owner, nChannels, freq);
//	return m_interface;
//}
//#endif  // NO_THEORA_PLAYER
