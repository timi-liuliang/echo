#include "openlabel.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
	OpenLabel::OpenLabel()
	{

	}

	OpenLabel::~OpenLabel()
	{

	}

	void OpenLabel::parse()
	{

	}

	void OpenLabel::save()
	{
		nlohmann::json openLabelJson;
		saveObjects(openLabelJson);

		nlohmann::json rootJson;
		rootJson["openlabel"] = openLabelJson;
	}

	void OpenLabel::saveObjects(nlohmann::json& parentJson)
	{
		nlohmann::json objectsJson;

		for(i32 i=0; i<m_objects.size(); i++)
		{
			LabelObject& object = m_objects[i];

			nlohmann::json objectJosn;
			objectJosn["name"] = "name";
			objectJosn["type"] = "type";


			objectsJson[StringUtil::ToString(i).c_str()] = objectJosn;
		}

		parentJson["objects"] = objectsJson;
	}
}