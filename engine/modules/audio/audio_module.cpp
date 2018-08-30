#include "audio_module.h"

namespace Echo
{
	AudioModule::AudioModule()
	{
		//FSAudioManager::instance()->init( 32, false);
		//FSAudioManager::instance()->loadAllBankFile();
	}

	void AudioModule::registerTypes()
	{
		//FSAudioManager::instance()->suspendFmodSystem();
		//FSAudioManager::instance()->resumeFmodSystem();
		// ÒôÆµ¹ÜÀíÆ÷
		//FSAudioManager::instance()->release();

		//EchoSafeDeleteInstance(FSAudioManager);
	}

	void AudioModule::update(float elapsedTime)
	{
		//FSAudioManager::instance()->tick(static_cast<ui32>(elapsedTime));
	}
}
