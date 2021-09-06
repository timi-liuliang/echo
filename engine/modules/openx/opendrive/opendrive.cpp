#include "opendrive.h"
#include "opendrive_spiral.h"
#include "opendrive_debug_draw.h"
#include "opendrive_module.h"
#include "engine/core/io/io.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	void OpenDrive::Geometry::getHeading(double& x, double& y)
	{
		x = cos(m_hdg);
		y = sin(m_hdg);
	}

	void OpenDrive::Geometry::getStart(double& x, double& y)
	{
		x = m_x;
		y = m_y;
	}

	double OpenDrive::Geometry::getHdg() const
	{
		auto getAngleInInterval2PI = [](double angle)
		{
			double angle2 = std::fmod(angle, 2 * Math::PI);

			if (angle2 < 0)
			{
				angle2 += 2 * Math::PI;
			}
			else if (angle2 == -0)
			{
				angle2 = 0;
			}

			return angle2;
		};

		return getAngleInInterval2PI(m_hdg);
	}

	void OpenDrive::Line::evaluate(double sampleLength, double& x, double& y, double& h)
	{
		h = m_hdg;
		x = m_x + sampleLength * cos(h);
		y = m_y + sampleLength * sin(h);
	}

	void OpenDrive::Arc::evaluate(double sampleLength, double& x, double& y, double& h)
	{
		// Reference https://github.com/esmini/esmini/blob/cc6238ca1c0ade9aefb94a1e7f2e48bc143b8e1f/EnvironmentSimulator/Modules/RoadManager/RoadManager.cpp
		// Esmini's implementation is right too. but i don't understand it...
		// Finally. I implement it by https://gupea.ub.gu.se/bitstream/2077/23047/1/gupea_2077_23047_1.pdf
		double radius = getRadius();

		double centerX;
		double centerY;
		getCenter(centerX, centerY);

		double centralAngle = sampleLength / radius;

		if (m_curvature < 0)
		{
			x = centerX + cos(m_hdg + Math::PI_DIV2 - centralAngle) * radius;
			y = centerY + sin(m_hdg + Math::PI_DIV2 - centralAngle) * radius;
			h = m_hdg + centralAngle;
		}
		else
		{
			x = centerX + cos(m_hdg - Math::PI_DIV2 + centralAngle) * radius;
			y = centerY + sin(m_hdg - Math::PI_DIV2 + centralAngle) * radius;
			h = m_hdg - centralAngle;
		}
	}

	// Watch out opendrive's inertial system
	// https://www.asam.net/index.php?eID=dumpFile&t=f&f=4422&token=e590561f3c39aa2260e5442e29e93f6693d1cccd#top-3e20902e-5698-44b3-8445-6ceb7849021f
	void OpenDrive::Arc::getCenter(double& x, double& y)
	{
		double radius = getRadius();

		if (m_curvature < 0)
		{
			x = m_x + cos(m_hdg - Math::PI_DIV2) * radius;
			y = m_y + sin(m_hdg - Math::PI_DIV2) * radius;
		}
		else
		{
			x = m_x + cos(m_hdg + Math::PI_DIV2) * radius;
			y = m_y + sin(m_hdg + Math::PI_DIV2) * radius;
		}
	}

	OpenDrive::Spiral::Spiral(double s, double x, double y, double hdg, double length, double curvatureStart, double curvatureEnd)
		: Geometry(s, x, y, hdg, length, Geometry::Spiral)
		, m_curvatureStart(curvatureStart)
		, m_curvatureEnd(curvatureEnd)
	{
		m_cdot = (m_curvatureEnd - m_curvatureStart) / m_length;
		if (std::fabs(m_cdot) < FLT_EPSILON)
		{
			if (std::fabs(m_curvatureStart) < FLT_EPSILON)
			{
				// Line
				m_line = EchoNew(OpenDrive::Line(s, x, y, hdg, length));
			}
			else
			{
				// Arc
				m_arc = EchoNew(OpenDrive::Arc(s, x, y, hdg, length, curvatureStart));
			}
		}
		else
		{
			if (std::fabs(m_curvatureStart) > FLT_EPSILON)
			{
				// Not starting from zero curvature (straight line)
				// How long do we need to follow the spiral to reach start curve value
				m_s0 = m_curvatureStart / m_cdot;

				// Find out x, y, heading of start position
				odrSpiral(m_s0, m_cdot, &m_x0, &m_y0, &m_h0);
			}
		}
	}

	OpenDrive::Spiral::~Spiral()
	{
		EchoSafeDelete(m_line, Line);
		EchoSafeDelete(m_arc, Arc);
	}

	void OpenDrive::Spiral::evaluate(double ds, double& x, double& y, double& h)
	{
		if (m_line)
		{
			m_line->evaluate(ds, x, y, h);
		}
		else if (m_arc)
		{
			m_arc->evaluate(ds, x, y, h);
		}
		else
		{
			double xTemp, yTemp, hTemp;
			odrSpiral(m_s0 + ds, m_cdot, &xTemp, &yTemp, &hTemp);

			// heading
			h = hTemp - m_h0 + getHdg();

			// location
			double x1 = xTemp - m_x0;
			double y1 = yTemp - m_y0;
			double x2 = x1 * cos(-m_h0) - y1 * sin(-m_h0);
			double y2 = x1 * sin(-m_h0) + y1 * cos(-m_h0);

			x = m_x + x2 * cos(getHdg()) - y2 * sin(getHdg());
			y = m_y + x2 * sin(getHdg()) + y2 * cos(getHdg());
		}
	}

	void OpenDrive::Spiral::getHeading(double& x, double& y)
	{
		if (m_line)
		{
			m_line->getHeading(x, y);
		}
		else if (m_arc)
		{
			m_arc->getHeading(x, y);
		}
		else
		{
			x = cos(m_h0);
			y = sin(m_h0);
		}
	}

	void OpenDrive::Spiral::getStart(double& x, double& y)
	{
		if (m_line || m_arc)
		{
			x = m_x;
			y = m_y;
		}
		else
		{
			x = m_x0;
			y = m_y0;
		}
	}

	void OpenDrive::Poly3::evaluate(double ds, double& x, double& y, double& h)
	{

	}

	void OpenDrive::ParamPoly3::evaluate(double ds, double& x, double& y, double& h)
	{

	}

	OpenDrive::OpenDrive()
		: Node()
	{
		m_debugDraw = EchoNew(OpenDriveDebugDraw);
	}

	OpenDrive::~OpenDrive()
	{
		reset();

		EchoSafeDelete(m_debugDraw, OpenDriveDebugDraw);
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
					double curvatureStart = typeNode.attribute("curvStart").as_double();
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
			m_debugDraw->setEnable(true);
			m_debugDraw->onDriveChanged(this);
		}
		else
		{
			m_debugDraw->setEnable(false);
		}
	}

	void OpenDrive::updateInternal(float elapsedTime)
	{
		m_debugDraw->update(elapsedTime);
	}
}