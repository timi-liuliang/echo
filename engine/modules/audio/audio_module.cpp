#include "audio_module.h"
#include "audio_player.h"
#include "audio_listener.h"
#include "audio_device.h"
#include "editor/audio_player_editor.h"
#include "editor/audio_listener_editor.h"

namespace Echo
{
	DECLARE_MODULE(AudioModule)

	AudioModule::AudioModule()
	{

	}

	AudioModule::~AudioModule()
	{
		EchoSafeDeleteInstance(AudioDevice);
	}

	AudioModule* AudioModule::instance()
	{
		static AudioModule* inst = EchoNew(AudioModule);
		return inst;
	}

	void AudioModule::bindMethods()
	{

	}

	void AudioModule::registerTypes()
	{
		Class::registerType<AudioPlayer>();
		Class::registerType<AudioListener>();
		Class::registerType<AudioDevice>();

		REGISTER_OBJECT_EDITOR(AudioPlayer, AudioPlayerEditor)
		REGISTER_OBJECT_EDITOR(AudioListener, AudioListenerEditor)
	}

	void AudioModule::update(float elapsedTime)
	{
		AudioDevice::instance()->step(elapsedTime);
	}
}
