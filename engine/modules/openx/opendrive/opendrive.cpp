#include "opendrive.h"
#include "opendrive_module.h"
#include "engine/core/io/io.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	OpenDrive::OpenDrive()
	{

	}

	OpenDrive::~OpenDrive()
	{
		reset();
	}

	void OpenDrive::bindMethods()
	{
		CLASS_BIND_METHOD(OpenDrive, getXodrRes, DEF_METHOD("getXodrRes"));
		CLASS_BIND_METHOD(OpenDrive, setXodrRes, DEF_METHOD("setXodrRes"));

		CLASS_REGISTER_PROPERTY(OpenDrive, "Xodr", Variant::Type::ResourcePath, "getXodrRes", "setXodrRes");
	}

	void OpenDrive::reset()
	{
		for (Road& road : m_roads)
		{
			EchoSafeDeleteContainer(road.m_geometries, Geometry);
		}

		m_roads.clear();
	}

	void OpenDrive::setXodrRes(const ResourcePath& path)
	{
		if (m_xodrRes.setPath(path.getPath()))
		{
			reset();

			parseXodr(IO::instance()->loadFileToString(path.getPath()));
		}
	}

	void OpenDrive::parseXodr(const String& content)
	{
		pugi::xml_document doc;
		doc.load_string(content.c_str());

		pugi::xml_node openDriveNode = doc.child("OpenDRIVE");
		for (pugi::xml_node roadNode = openDriveNode.child("road"); roadNode; roadNode = roadNode.next_sibling("road"))
		{
			Road road;
			road.m_name = roadNode.attribute("name").as_string();
			road.m_length = roadNode.attribute("length").as_double();
			road.m_id = roadNode.attribute("id").as_int();
			road.m_junction = roadNode.attribute("junction").as_int();

			// Parse geometry
			parseGeometry(road, roadNode);

			m_roads.emplace_back(road);
		}
	}

	void OpenDrive::parseGeometry(Road& road, pugi::xml_node roadNode)
	{
		pugi::xml_node planViewNode = roadNode.child("planView");
		if (planViewNode)
		{
			for (pugi::xml_node geometryNode = planViewNode.child("geometry"); geometryNode; geometryNode = geometryNode.next_sibling())
			{
				double s	  = geometryNode.attribute("s").as_double();
				double x	  = geometryNode.attribute("x").as_double();
				double y	  =	geometryNode.attribute("y").as_double();
				double hdg	  =	geometryNode.attribute("hdg").as_double();
				double length = geometryNode.attribute("length").as_double();

				pugi::xml_node typeNode = geometryNode.last_child();
				if (StringUtil::Equal(typeNode.name(), "line"))
				{
					road.m_geometries.push_back(EchoNew(Line(s, x, y, hdg, length)));
				}
				else if (StringUtil::Equal(typeNode.name(), "arc"))
				{
	
				}
				else if (StringUtil::Equal(typeNode.name(), "spiral"))
				{

				}
				else if (StringUtil::Equal(typeNode.name(), "poly3"))
				{

				}
				else if (StringUtil::Equal(typeNode.name(), "paramPoly3"))
				{

				}
			}
		}
	}

	void OpenDrive::setDebugDrawOption(const StringOption& option)
	{
		m_debugDrawOption = option.toEnum(DebugDrawOption::Editor);
	}

	void OpenDrive::updateInternal(float elapsedTime)
	{
		if ((m_debugDrawOption == DebugDrawOption::All) ||
			(m_debugDrawOption == DebugDrawOption::Editor && !IsGame) ||
			(m_debugDrawOption == DebugDrawOption::Game && IsGame))
		{
			m_debugDraw.setEnable(true);
			m_debugDraw.update(elapsedTime, this);
		}
		else
		{
			m_debugDraw.setEnable(false);
		}
	}
}