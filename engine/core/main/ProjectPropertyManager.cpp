#include "ProjectPropertyManager.h"
#include "engine/core/resource/DataStream.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_helper.hpp"

namespace Echo
{

	ProjectPropertyManager::ProjectPropertyManager()
	{

	}

	ProjectPropertyManager::~ProjectPropertyManager()
	{

	}

	void ProjectPropertyManager::init()
	{
		MemoryReader memory = MemoryReader("project.pro");
		char* data = memory.getData<char*>();
		if (!data)
		{
			return;
		}

		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* root = doc.first_node("ProjectPropertys");
		rapidxml::xml_node<>* tags = root->first_node("Tags");
		if (tags)
		{
			rapidxml::xml_node<>* tagNode = tags->first_node("Tag");
			while (tagNode)
			{
				rapidxml::xml_attribute<>* nameAttr = tagNode->first_attribute("name");
				String name = nameAttr->value();
				if (!name.empty())
				{
					addTag(name);
				}
				tagNode = tagNode->next_sibling();
			}
		}
	}

	bool ProjectPropertyManager::findTag(const String& tag)
	{
		if (std::find(m_tagList.begin(), m_tagList.end(), tag) == m_tagList.end())
		{
			return false;
		}

		return true;
	}

	bool ProjectPropertyManager::addTag(const String& tag)
	{
		if (findTag(tag))
		{
			return false;
		}

		m_tagList.push_back(tag);
		return true;
	}

	bool ProjectPropertyManager::removeTag(const String& tag)
	{
		auto it = std::find(m_tagList.begin(), m_tagList.end(), tag);
		if (it == m_tagList.end())
		{
			return false;
		}

		if (m_tagObjectList.find(tag) != m_tagObjectList.end())
		{
			return false;
		}

		m_tagList.erase(it);
		return true;
	}

	const StringArray& ProjectPropertyManager::getTagList()
	{
		return m_tagList;
	}

	void ProjectPropertyManager::addObjectWithTag(const String& tag, const String& objName)
	{
		m_tagObjectList.insert(make_pair(tag, objName));
	}

	void ProjectPropertyManager::removeObjectWithTag(const String& tag, const String& objName)
	{
		auto range = m_tagObjectList.equal_range(tag);
		for (auto it = range.first; it != range.second;)
		{
			if (it->second == objName)
			{
				it = m_tagObjectList.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

}