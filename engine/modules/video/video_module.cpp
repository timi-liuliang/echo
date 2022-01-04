#include "video_module.h"
#include "video_capture.h"
#include "video_player.h"
#include "editor/video_capture_editor.h"
#include "editor/video_player_editor.h"

namespace Echo
{
	DECLARE_MODULE(VideoModule)

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
		Class::registerType<VideoCapture>();
		Class::registerType<VideoPlayer>();

		CLASS_REGISTER_EDITOR(VideoCapture, VideoCaptureEditor)
		CLASS_REGISTER_EDITOR(VideoPlayer, VideoPlayerEditor)
	}

	void VideoModule::update(float elapsedTime)
	{
	}
}
