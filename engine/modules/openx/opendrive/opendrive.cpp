#include "opendrive.h"
#include "opendrive_module.h"
#include "engine/core/io/io.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	void OpenDrive::Line::evaluate(double sampleLength, double& x, double& y, double& h)
	{
		h = m_hdg;
		x = m_x + sampleLength * cos(h);
		y = m_y + sampleLength * sin(h);
	}

	void OpenDrive::Arc::evaluate(double sampleLength, double& x, double& y, double& h)
	{
		double xLocal = 0;
		double yLocal = 0;

		// arcLength = angle * radius -> angle = arcLength / radius -> angle = arcLength * curvature
		double angle = sampleLength * m_curvature;
		double radius = getRadius();

		if (m_curvature < 0)
		{
			// starting from 90 degrees going clockwise
			xLocal = cos(angle + Math::PI_DIV2);
			yLocal = sin(angle + Math::PI_DIV2) - 1.f; // -1 transform to y = 0
		}
		else
		{
			// starting from -90 degrees going counter clockwise
			xLocal = cos(angle + 3.0 * Math::PI_DIV2);
			yLocal = sin(angle + 3.0 * Math::PI_DIV2) + 1;  // +1 transform to y = 0
		}

		x = m_x + radius * (xLocal * cos(m_hdg) - yLocal * sin(m_hdg));
		y = m_y + radius * (xLocal * sin(m_hdg) + yLocal * cos(m_hdg));
		h = m_hdg + angle;

		//double radius = getRadius();

		//double centerX;
		//double centerY;
		//getCenter(centerX, centerY);

		//double centralAngle = sampleLength / radius;

		//if (m_curvature < 0)
		//{
		//	x = centerX + cos(m_hdg + Math::PI_DIV2 + centralAngle) * radius;
		//	y = centerY + sin(m_hdg + Math::PI_DIV2 + centralAngle) * radius;
		//	h = m_hdg + centralAngle;
		//}
		//else
		//{
		//	x = centerX + cos(-m_hdg - Math::PI_DIV2 - centralAngle) * radius;
		//	y = centerY + sin(-m_hdg - Math::PI_DIV2 - centralAngle) * radius;
		//	h = m_hdg - centralAngle;
		//}
	}

	void OpenDrive::Arc::getCenter(double& x, double& y)
	{
		double radius = getRadius();

		if (m_curvature < 0)
		{
			x = m_x + cos(m_hdg + Math::PI_DIV2 - Math::PI) * radius;
			y = m_y + sin(m_hdg + Math::PI_DIV2 - Math::PI) * radius;
		}
		else
		{
			x = m_x + cos(m_hdg - Math::PI_DIV2 - Math::PI) * radius;
			y = m_y + sin(m_hdg - Math::PI_DIV2 - Math::PI) * radius;
		}
	}

	void OpenDrive::Spiral::evaluate(double ds, double& x, double& y, double& h)
	{

	}

	void OpenDrive::Poly3::evaluate(double ds, double& x, double& y, double& h)
	{

	}

	void OpenDrive::ParamPoly3::evaluate(double ds, double& x, double& y, double& h)
	{

	}

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

		refreshDebugDraw();
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
					double curvature = typeNode.attribute("curvature").as_double();
					road.m_geometries.push_back(EchoNew(Arc(s, x, y, hdg, length, curvature)));
				}
				else if (StringUtil::Equal(typeNode.name(), "spiral"))
				{
					double curvatureStart = typeNode.attribute("curvStatrt").as_double();
					double curvatureEnd = typeNode.attribute("curvEnd").as_double();
					road.m_geometries.push_back(EchoNew(Spiral(s, x, y, hdg, length, curvatureStart, curvatureEnd)));
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

		refreshDebugDraw();
	}

	void OpenDrive::refreshDebugDraw()
	{
		if ((m_debugDrawOption == DebugDrawOption::All) ||
			(m_debugDrawOption == DebugDrawOption::Editor && !IsGame) ||
			(m_debugDrawOption == DebugDrawOption::Game && IsGame))
		{
			m_debugDraw.setEnable(true);
			m_debugDraw.onDriveChanged(this);
		}
		else
		{
			m_debugDraw.setEnable(false);
		}
	}

	void OpenDrive::updateInternal(float elapsedTime)
	{
		m_debugDraw.update(elapsedTime);
	}
}