#pragma once
#include "engine/core/Memory/MemAllocDef.h"

namespace Echo
{
	class ProjectPropertyManager
	{
		friend class GameObject;
	public:
		ProjectPropertyManager();
		~ProjectPropertyManager();

		void init();

		bool findTag(const String& tag);

		bool addTag(const String& tag);

		bool removeTag(const String& tag);

		const StringArray& getTagList();

	private:
		void addObjectWithTag(const String& tag, const String& objName);
		void removeObjectWithTag(const String& tag, const String& objName);

		StringArray m_tagList;
		multimap<String, String>::type m_tagObjectList;
	};
}