#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/util/base64.h"
#include "engine/modules/anim/anim_clip.h"
#include "gltf_res.h"

namespace Echo
{
	class GltfSkeleton : public Node
	{
		ECHO_CLASS(GltfSkeleton, Node)

	public:
		GltfSkeleton();
		virtual ~GltfSkeleton();

		// set gltf resource
		const ResourcePath& getGltfRes() { return m_assetPath; }
		void setGltfRes(const ResourcePath& path);

		// play anim
		void setAnim(const StringOption& animName);

		// get animations
		const StringOption& getAnim() { return m_animations; }

		// get current anim clip
		const AnimClip* getAnimClip();

		// is anim exist
		bool isAnimExist(const char* animName);

		// get node transform
		bool getGltfNodeTransform(Transform& transform, size_t nodeIdx);

	protected:
		// update self
		virtual void update_self() override;

	private:
		// generate unique name
		void generateUniqueName(String& oName);

		//  query clip data
		void extractClipData(AnimClip* clip);

		// joint transform
		void jointInhertParentTransform(i32 parent);

	private:
		ResourcePath					m_assetPath;
		GltfResPtr						m_asset;			// gltf asset ptr
		StringOption					m_animations;
		vector<AnimClip*>::type			m_clips;
		vector<Node::Transform>::type	m_nodeTransforms;
	};
}
