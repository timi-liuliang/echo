#include "anim_player.h"

namespace Echo
{
	AnimPlayer::AnimPlayer()
	{
	}

	AnimPlayer::~AnimPlayer()
	{
	}

	void AnimPlayer::bindMethods()
	{
		CLASS_BIND_METHOD(AnimPlayer, getAnimData, DEF_METHOD("getAnimData"));
		CLASS_BIND_METHOD(AnimPlayer, setAnimData, DEF_METHOD("setAnimData"));

		CLASS_REGISTER_PROPERTY(AnimPlayer, "AnimData", Variant::Type::Base64String, "getAnimData", "setAnimData");
	}
}