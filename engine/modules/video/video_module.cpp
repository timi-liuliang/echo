#include "video_module.h"
#include "video_player.h"
#include "editor/video_player_editor.h"

namespace Echo
{
	VideoModule::VideoModule()
	{

	}

	VideoModule::~VideoModule()
	{
	}

	VideoModule* VideoModule::instance()
	{
		static VideoModule* inst = EchoNew(VideoModule);
		return inst;
	}

	void VideoModule::bindMethods()
	{

	}

	void VideoModule::registerTypes()
	{
		Class::registerType<VideoPlayer>();

		REGISTER_OBJECT_EDITOR(VideoPlayer, VideoPlayerEditor)
	}

	void VideoModule::update(float elapsedTime)
	{
	}
}
