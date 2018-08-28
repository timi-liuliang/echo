#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/util/base64.h"
#include "engine/modules/anim/anim_clip.h"

namespace Echo
{
	class GltfSkeleton : public Node
	{
		ECHO_CLASS(GltfSkeleton, Node)

	public:
		GltfSkeleton();
		virtual ~GltfSkeleton();

		// play anim
		void setAnim(const StringOption& animName);

		// get animations
		const StringOption& getAnim() { return m_animations; }

		// is anim exist
		bool isAnimExist(const char* animName);

	public:
		// add clip
		void addClip(AnimClip* clip);

	protected:
		// update self
		virtual void update_self() override;

	private:
		// generate unique name
		void generateUniqueName(String& oName);

	private:
		vector<AnimClip*>::type	m_clips;
		StringOption			m_animations;
	};
}
