#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/util/base64.h"
#include "anim_clip.h"

namespace Echo
{
	class Timeline : public Node
	{
		ECHO_CLASS(Timeline, Node)

	public:
		// object types
		enum ObjectType
		{
			Node,
			Setting,
			Resource,
		};

	public:
		Timeline();
		virtual ~Timeline();

		// play anim
		void setAnim(const StringOption& animName);

		// get animations
		const StringOption& getAnim() { return m_animations; }

		// is anim exist
		bool isAnimExist(const char* animName);

		// get anim data
		const Base64String& getAnimData() const { return m_animData; }

		// set anim data
		void setAnimData(const Base64String& data) { m_animData = data; }

	public:
		// clip count
		int  getClipCount() const { return static_cast<int>(m_clips.size()); }

		// get clip by index
		AnimClip* getClip(int idx) { return m_clips[idx]; }

		// get clip by name
		AnimClip* getClip(const char* animName);

		// get clip index
		int getClipIndex(const char* animName);

		// add clip
		void addClip(AnimClip* clip);

		// delete clip
		void deleteClip(const char* animName);

		// rename clip
		void renameClip(int idx, const char* newName);

	public:
		// add object to animclip
		void addObject(ObjectType type, const String& path);

	protected:
		// update self
		virtual void update_self() override;

	public:
		// generate unique name
		void generateUniqueAnimName(const String& prefix, String& oName);

	private:
		Base64String			m_animData;
		vector<AnimClip*>::type	m_clips;
		StringOption			m_animations;
	};
}
