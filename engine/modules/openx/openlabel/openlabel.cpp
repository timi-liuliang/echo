#include "openlabel.h"
#include "openlabel_debug_draw.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
	OpenLabel::OpenLabel()
	{
		m_debugDraw = EchoNew(OpenLabelDebugDraw);
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

		CLASS_REGISTER_PROPERTY(OpenDrive, "Res", Variant::Type::ResourcePath, getRes, setRes);
		CLASS_REGISTER_PROPERTY(OpenDrive, "DebugDraw", Variant::Type::Object, getDebugDraw, setDebugDraw);
		CLASS_REGISTER_PROPERTY_HINT(OpenDrive, "DebugDraw", PropertyHintType::ObjectType, "OpenLabelDebugDraw");
	}

	void OpenLabel::parse()
	{

	}

	void OpenLabel::reset()
	{
		m_objects.clear();
	}

	void OpenLabel::setRes(const ResourcePath& path)
	{
		if (m_resPath.setPath(path.getPath()))
		{

		}
	}

	void OpenLabel::setDebugDraw(Object* debugDraw)
	{
		EchoSafeDelete(m_debugDraw, OpenLabelDebugDraw);
		m_debugDraw = (OpenLabelDebugDraw*)debugDraw;

		refreshDebugDraw();
	}

	void OpenLabel::refreshDebugDraw()
	{
		if (m_debugDraw)
		{
			m_debugDraw->setEnable(true);
			m_debugDraw->onOpenLabelChanged(this);
		}
	}

	void OpenLabel::updateInternal(float elapsedTime)
	{
		//if (m_xodrDirty)
		//{
		//	reset();
		//	parseXodr(IO::instance()->loadFileToString(m_xodrRes.getPath()));

		//	m_xodrDirty = false;
		//}

		if (m_debugDraw)
			m_debugDraw->update(elapsedTime);
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