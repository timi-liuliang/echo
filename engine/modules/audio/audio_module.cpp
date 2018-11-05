#include "audio_module.h"
#include "audio_player.h"
#include "editor/audio_player_editor.h"

namespace Echo
{
	AudioModule::AudioModule()
	{

	}

	void AudioModule::registerTypes()
	{
		Class::registerType<AudioPlayer>();

		REGISTER_OBJECT_EDITOR(AudioPlayer, AudioPlayerEditor)
	}

	void AudioModule::update(float elapsedTime)
	{
	}
}
