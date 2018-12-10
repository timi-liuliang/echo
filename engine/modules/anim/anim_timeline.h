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
		// play state
		enum class PlayState
		{
			Playing,
			Pause,
			Stop,
		};

		// object types
		enum ObjectType
		{
			Node,
			Setting,
			Resource,
		};

		struct ObjectUserData
		{
			ObjectType	m_type;
			String		m_path;

			ObjectUserData(ObjectType type, const String& path)
				: m_type(type), m_path(path)
			{}
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
		const Base64String& getAnimData();

		// set anim data
		void setAnimData(const Base64String& data);

	public:
		// get play state
		PlayState getPlayState() const { return m_playState; }

		// play animation
		void play(const char* animName);
		void pause();
		void stop();

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
		void addObject(const String& animName, ObjectType type, const String& path);

	public:
		//property
		AnimProperty* getProperty(const String& animName, const String& objectPath, const String& propertyName);
		void addProperty(const String& animName, const String& objectPath, const String& propertyName);

		// add key frame
		void addKey(const String& animName, const String& objectPath, const String& propertyName, float time, const Variant& value);

	protected:
		// update self
		virtual void update_self() override;

	public:
		// generate unique name
		void generateUniqueAnimName(const String& prefix, String& oName);

		// apply clip
		void extractClipData(AnimClip* clip);

	private:
		PlayState				m_playState;
		vector<AnimClip*>::type	m_clips;
		Base64String			m_animData;
		bool					m_isAnimDataDirty;
		StringOption			m_animations;
	};
}
