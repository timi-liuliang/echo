#include "class.h"
#include <map>

namespace Echo
{
	static std::map<String, String>*	g_classInfos = nullptr;

	// add class
	void Class::addClass(const char* className, const char* parentName)
	{
		if (!g_classInfos)
		{
			g_classInfos = new std::map<String, String>();
		}

		(*g_classInfos)[className] = parentName;
	}

	// get all child class
	bool Class::getChildClasses(StringArray& childClasses, const char* className)
	{
		// keep clean
		childClasses.clear();

		if (!g_classInfos)
			return false;

		for (auto it : *g_classInfos)
		{
			if (it.second == className)
			{
				childClasses.push_back(it.first);
			}
		}

		return !childClasses.empty();
	}
}