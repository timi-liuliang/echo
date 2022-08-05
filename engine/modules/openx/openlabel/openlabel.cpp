#include "openlabel.h"
#include "openlabel_debug_draw.h"
#include "engine/core/io/io.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
	OpenLabel::OpenLabel()
	{
	}

	OpenLabel::~OpenLabel()
	{
		reset();

		EchoSafeDelete(m_debugDraw, OpenLabelDebugDraw);
	}

	void OpenLabel::bindMethods()
	{
		CLASS_BIND_METHOD(OpenLabel, getRes);
		CLASS_BIND_METHOD(OpenLabel, setRes);
		CLASS_BIND_METHOD(OpenLabel, getDebugDraw);
		CLASS_BIND_METHOD(OpenLabel, setDebugDraw);

		CLASS_REGISTER_PROPERTY(OpenLabel, "Res", Variant::Type::ResourcePath, getRes, setRes);
		CLASS_REGISTER_PROPERTY(OpenLabel, "DebugDraw", Variant::Type::Object, getDebugDraw, setDebugDraw);
		CLASS_REGISTER_PROPERTY_HINT(OpenLabel, "DebugDraw", PropertyHintType::ObjectType, "OpenLabelDebugDraw");
	}

	void OpenLabel::reset()
	{
		m_objects.clear();
	}

	void OpenLabel::setRes(const ResourcePath& path)
	{
		if (m_resPath.setPath(path.getPath()))
		{
			m_dirty = true;
		}
	}

	OpenLabelDebugDraw* OpenLabel::getDebugDraw() 
	{
		if (!m_debugDraw)
			m_debugDraw = EchoNew(OpenLabelDebugDraw);

		return m_debugDraw; 
	}

	void OpenLabel::setDebugDraw(Object* debugDraw)
	{
		EchoSafeDelete(m_debugDraw, OpenLabelDebugDraw);
		m_debugDraw = (OpenLabelDebugDraw*)debugDraw;

		refreshDebugDraw();
	}

	void OpenLabel::refreshDebugDraw()
	{
		if (getDebugDraw())
		{
			m_debugDraw->setEnable(true);
			m_debugDraw->onOpenLabelChanged(this);
		}
	}

	void OpenLabel::updateInternal(float elapsedTime)
	{
		if (m_dirty)
		{
			reset();
			parse(IO::instance()->loadFileToString(m_resPath.getPath()));

			m_dirty = false;
		}

		if (m_debugDraw)
			m_debugDraw->update(elapsedTime, this);
	}

	void OpenLabel::parse(const String& content)
	{
		if (!content.empty())
		{
			nlohmann::json j = nlohmann::json::parse(content);

			if (j.find("openlabel") != j.end())
			{
				parseObjects(j["openlabel"]);
			}
		}

		refreshDebugDraw();
	}

	void OpenLabel::parseObjects(nlohmann::json& json)
	{
		if (json.find("objects") != json.end())
		{
			nlohmann::json& objects = json["objects"];

			for (i32 i = 0; i < objects.size(); i++)
			{
				nlohmann::json& object = objects[StringUtil::ToString(i).c_str()];

				LabelObject labelObject;
				labelObject.m_name = object["name"].get<std::string>().c_str();
				labelObject.m_type = object["type"].get<std::string>().c_str();

				parsePoly2ds(object["poly2d"], labelObject);
				parseCuboid2ds(object["cuboid2d"], labelObject);

				m_objects.push_back(labelObject);
			}
		}
	}

	void OpenLabel::parsePoly2ds(nlohmann::json& parentJson, LabelObject& object)
	{
		for (i32 i = 0; i < parentJson.size(); i++)
		{
			nlohmann::json poly2dJson = parentJson[i];

			Poly2d poly2d;

			i32 pointCount = poly2dJson["val"].size() / 2;
			for (i32 j = 0; j < pointCount; j++)
			{
				Vector2 uv;
				uv.x = poly2dJson["val"][j * 2 + 0].get<float>();
				uv.y = poly2dJson["val"][j * 2 + 1].get<float>();

				bool visible = poly2dJson["visible"][j];

				poly2d.m_values.push_back(uv);
				poly2d.m_visibles.push_back(visible);
			}

			object.m_poly2ds.push_back(poly2d);
		}
	}

	void OpenLabel::parseCuboid2ds(nlohmann::json& parentJson, LabelObject& object)
	{
		for (i32 i = 0; i < parentJson.size(); i++)
		{
			nlohmann::json cuboid2dJson = parentJson[i];

			Cuboid2d cuboid2d;

			i32 pointCount = cuboid2dJson["val"].size() / 2;
			for (i32 j = 0; j < pointCount; j++)
			{
				Vector2 uv;
				uv.x = cuboid2dJson["val"][j * 2 + 0].get<float>();
				uv.y = cuboid2dJson["val"][j * 2 + 1].get<float>();

				bool visible = cuboid2dJson["visible"][j];

				cuboid2d.m_values.push_back(uv);
				cuboid2d.m_visibles.push_back(visible);
			}

			object.m_cuboid2ds.push_back(cuboid2d);
		}
	}

	void OpenLabel::save(const char* savePath)
	{
		nlohmann::json openLabelJson;
		saveObjects(openLabelJson);

		nlohmann::json rootJson;
		rootJson["openlabel"] = openLabelJson;

		std::ofstream file(savePath);
		file << std::setw(4) << rootJson << std::endl;
	}

	void OpenLabel::saveObjects(nlohmann::json& parentJson)
	{
		for (i32 i = 0; i < m_objects.size(); i++)
		{
			LabelObject& object = m_objects[i];

			nlohmann::json objectJosn;
			objectJosn["name"] = object.m_name;
			objectJosn["type"] = object.m_type;

			savePoly2ds(objectJosn, object);
			saveCuboid2ds(objectJosn, object);

			parentJson["objects"][StringUtil::ToString(i).c_str()] = objectJosn;
		}
	}

	void OpenLabel::savePoly2ds(nlohmann::json& parentJson, LabelObject& object)
	{
		for (size_t i = 0; i < object.m_poly2ds.size(); i++)
		{
			Poly2d& poly2d = object.m_poly2ds[i];

			nlohmann::json poly2dJson;
			for (Vector2& uv : poly2d.m_values)
			{
				poly2dJson["val"].push_back(uv.x);
				poly2dJson["val"].push_back(uv.y);
			}

			for (bool visible : poly2d.m_visibles)
			{
				poly2dJson["visible"].push_back(visible);
			}

			parentJson["poly2d"].push_back(poly2dJson);
		}
	}

	void OpenLabel::saveCuboid2ds(nlohmann::json& parentJson, LabelObject& object)
	{
		for (i32 i = 0; i < object.m_cuboid2ds.size(); i++)
		{
			Cuboid2d& cuboid2d = object.m_cuboid2ds[i];

			nlohmann::json cuboid2dJson;
			for (Vector2& uv : cuboid2d.m_values)
			{
				cuboid2dJson["val"].push_back(uv.x);
				cuboid2dJson["val"].push_back(uv.y);
			}

			for (bool visible : cuboid2d.m_visibles)
			{
				cuboid2dJson["visible"].push_back(visible);
			}

			parentJson["cuboid2d"].push_back(cuboid2dJson);
		}
	}
}