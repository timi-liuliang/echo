#include "opendrive.h"
#include "opendrive_spiral.h"
#include "opendrive_debug_draw.h"
#include "opendrive_module.h"
#include "engine/core/io/io.h"
#include "engine/core/log/Log.h"
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

	void OpenDrive::Arc::evaluate(double ds, double& x, double& y, double& h)
	{
		// Reference https://github.com/esmini/esmini/blob/cc6238ca1c0ade9aefb94a1e7f2e48bc143b8e1f/EnvironmentSimulator/Modules/RoadManager/RoadManager.cpp
		// Esmini's implementation is right too. but i don't understand it...
		// Finally. I implement it by https://gupea.ub.gu.se/bitstream/2077/23047/1/gupea_2077_23047_1.pdf
		double radius = getRadius();

		double centerX;
		double centerY;
		getCenter(centerX, centerY);

		double centralAngle = ds / radius;

		if (m_curvature < 0)
		{
			x = centerX + cos(m_hdg + Math::PI_DIV2 - centralAngle) * radius;
			y = centerY + sin(m_hdg + Math::PI_DIV2 - centralAngle) * radius;
		}
		else
		{
			x = centerX + cos(m_hdg - Math::PI_DIV2 + centralAngle) * radius;
			y = centerY + sin(m_hdg - Math::PI_DIV2 + centralAngle) * radius;
		}

		h = getHdg() + ds * m_curvature;
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

	OpenDrive::Polynomial::Polynomial(double a, double b, double c, double d, double scale)
		: m_a(a)
		, m_b(b)
		, m_c(c)
		, m_d(d)
		, m_scale(scale)
	{

	}

	void OpenDrive::Polynomial::set(double a, double b, double c, double d, double scale)
	{
		m_a = a;
		m_b = b;
		m_c = c;
		m_d = d;
		m_scale = scale;
	}

	double OpenDrive::Polynomial::evaluate(double p)
	{
		p *= m_scale;

		return (m_a + m_b * p + m_c * p * p + m_d * p * p * p);
	}

	double OpenDrive::Polynomial::evaluatePrim(double p)
	{
		p *= m_scale;

		return (m_b + 2 * m_c * p + 3 * m_d * p * p);
	}

	double OpenDrive::Polynomial::evaluatePrimPrim(double p)
	{
		p *= m_scale;

		return (2 * m_c + 6 * m_d * p);
	}

	OpenDrive::Poly3::Poly3(double s, double x, double y, double hdg, double length, double a, double b, double c, double d)
		: Geometry(s, x, y, hdg, length, Geometry::Poly3)
	{
		m_poly3.set(a, b, c, d);

		double xTmp = 0.0;
		double yTmp = 0.0;

		evaluateDsLocal(m_length - FLT_EPSILON, xTmp, yTmp);
		m_uMax = xTmp;
	}

	void OpenDrive::Poly3::evaluate(double ds, double& x, double& y, double& h)
	{
		double uLocal = 0.0;
		double vLocal = 0.0;

		evaluateDsLocal(ds, uLocal, vLocal);

		x = m_x + uLocal * cos(getHdg()) - vLocal * sin(getHdg());
		y = m_y + uLocal * sin(getHdg()) + vLocal * cos(getHdg());
		h = getHdg() + atan(m_poly3.evaluatePrim(uLocal));
	}

	void OpenDrive::Poly3::evaluateDsLocal(double ds, double& u, double& v)
	{
		double distTmp = 0.0;
		double stepLen = std::min<double>(10, ds);

		u = v = 0.0;

		if (ds > m_length - FLT_EPSILON)
		{
			u = m_uMax;
			v = m_poly3.evaluate(u);
		}
		else if (ds > FLT_EPSILON)
		{
			for (double uTmp = 0; uTmp < m_length; uTmp += stepLen)
			{
				double vTmp = m_poly3.evaluate(uTmp);
				double delta = sqrt((uTmp - u) * (uTmp - u) + (vTmp - v) * (vTmp - v));

				if (distTmp + delta > ds)
				{
					double w = (distTmp + delta - ds) / std::max<double>(delta, FLT_EPSILON);
					u = w * u + (1.0 - w) * uTmp;
					v = m_poly3.evaluate(u);

					break;
				}

				distTmp += delta;
				u = uTmp;
				v = vTmp;
			}
		}
	}

	void OpenDrive::ParamPoly3::calcs2pMap(OpenDrive::ParamPoly3::RangeType type)
	{
		double len = 0;
		double stepLen = 1.0 / double(PARAMPOLY3_STEPS);
		double p = 0;

		if (type == OpenDrive::ParamPoly3::RangeType::ArcLength)
		{
			stepLen = m_length / PARAMPOLY3_STEPS;
		}

		// Calculate actual arc length of the curve
		m_s2pMap[0][0] = 0;
		for (size_t i = 1; i < PARAMPOLY3_STEPS + 1; i++)
		{
			p += stepLen;

			double pm = p - 0.5 * stepLen; // midpoint method
			double integrator = sqrt(
				pow(3 * m_poly3U.m_d * pm * pm + 2 * m_poly3U.m_c * pm + m_poly3U.m_b, 2) +
				pow(3 * m_poly3V.m_d * pm * pm + 2 * m_poly3V.m_c * pm + m_poly3V.m_b, 2));

			len += stepLen * integrator;
			m_s2pMap[i][0] = len;
		}

		// Map length (ds) to p for each sub-segment, adjust for incorrect length attribute
		double scaleFactor = m_length / len;

		for (size_t i = 0; i < PARAMPOLY3_STEPS + 1; i++)
		{
			m_s2pMap[i][0] *= scaleFactor;
			m_s2pMap[i][1] = i * m_length / PARAMPOLY3_STEPS;
		}
	}

	double OpenDrive::ParamPoly3::s2p(double s)
	{
		for (size_t i = 0; i < PARAMPOLY3_STEPS; i++)
		{
			if (m_s2pMap[i + 1][0] > s)
			{
				double w = (s - m_s2pMap[i][0]) / (m_s2pMap[i + 1][0] - m_s2pMap[i][0]);
				return m_s2pMap[i][1] + w * (m_s2pMap[i + 1][1] - m_s2pMap[i][1]);
			}
		}

		return m_s2pMap[PARAMPOLY3_STEPS][1];
	}

	void OpenDrive::ParamPoly3::evaluate(double ds, double& x, double& y, double& h)
	{
		double p = s2p(ds);
		double hdg = getHdg();

		double uLocal = m_poly3U.evaluate(p);
		double vLocal = m_poly3V.evaluate(p);

		x = m_x + uLocal * cos(hdg) - vLocal * sin(hdg);
		y = m_y + uLocal * sin(hdg) + vLocal * cos(hdg);
		h = hdg + atan2(m_poly3V.evaluatePrim(p), m_poly3U.evaluatePrim(p));
	}

	OpenDrive::LaneWidth* OpenDrive::Lane::getWidth(double s)
	{
		if (!m_widthes.empty())
		{
			for (int i = 0; i + 1 < int(m_widthes.size()); i++)
			{
				if (s < m_widthes[i].m_offset)
					return &m_widthes[i];
			}

			return &m_widthes.back();
		}

		return nullptr;
	}

	OpenDrive::Lane* OpenDrive::LaneSection::getLaneById(i32 laneId)
	{
		for (Lane& lane : m_lanes)
		{
			if (lane.m_id == laneId)
				return &lane;
		}

		return nullptr;
	}

	double OpenDrive::LaneSection::getLaneWidth(double s, int laneId)
	{
		// Reference lane has no width
		if (laneId == 0)
			return 0.0;

		// Make sure s in secton range
		s = std::clamp<double>(s, m_s, m_s + m_length);

		Lane* lane = getLaneById(laneId);
		if (!lane)
			return 0.0;

		LaneWidth* laneWidth = lane->getWidth(s - m_s);
		if (!laneWidth)
			return 0.0;

		double ds = s - (m_s + laneWidth->m_offset);
		return laneWidth->m_poly3.evaluate(ds);
	}

	double OpenDrive::LaneSection::getLaneInnerOffset(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		int step = laneId < 0 ? +1 : -1;
		return getLaneOuterOffset(s, laneId+step);
	}

	double OpenDrive::LaneSection::getLaneCenterOffset(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		double outerOffset = getLaneOuterOffset(s, laneId);
		double width = getLaneWidth(s, laneId);

		return outerOffset - width / 2.0;
	}

	double OpenDrive::LaneSection::getLaneOuterOffset(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		double width = getLaneWidth(s, laneId);
		if (std::abs(laneId) == 1)
		{
			// this lane is next to reference line.
			return width;
		}
		else
		{
			int innerLaneId = laneId + (laneId < 0 ? +1 : -1);
			return (width + getLaneOuterOffset(s, innerLaneId));
		}
	}

	double OpenDrive::LaneSection::getLaneInnerOffsetHeading(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		int step = laneId < 0 ? +1 : -1;
		return getLaneOuterOffsetHeading(s, laneId + step);
	}

	double OpenDrive::LaneSection::getLaneCenterOffsetHeading(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		double innerHeading = getLaneInnerOffsetHeading(s, laneId);
		double outerHeading = getLaneCenterOffsetHeading(s, laneId);

		return (innerHeading + outerHeading) * 0.5;
	}

	double OpenDrive::LaneSection::getLaneOuterOffsetHeading(double s, int laneId)
	{
		if (laneId == 0)
			return 0.0;

		Lane* lane = getLaneById(laneId);
		if (!lane)
			return 0.0;

		LaneWidth* laneWidth = lane->getWidth(s - m_s);
		if (!laneWidth)
			return 0.0;

		double ds = s - (m_s + laneWidth->m_offset);
		double heading = laneWidth->m_poly3.evaluatePrim(ds);

		if (std::abs(laneId) == 1)
		{
			return heading;
		}
		else
		{
			int innerLaneId = laneId + (laneId < 0 ? +1 : -1);
			return (heading + getLaneOuterOffsetHeading(s, innerLaneId));
		}
	}

	OpenDrive::Geometry* OpenDrive::Road::getGeometryByS(double ds)
	{
		if (!m_geometries.empty())
		{
			for (int i = 0; i + 1 < int(m_geometries.size()); i++)
			{
				if (ds < m_geometries[i]->m_s)
					return m_geometries[i];
			}

			if(ds<=m_length)
				return m_geometries.back();
		}

		return nullptr;
	}

	OpenDrive::LaneSection* OpenDrive::Road::getLaneSectionByS(double ds)
	{
		if (!m_laneSections.empty())
		{
			for (int i = 0; i + 1 < int(m_laneSections.size()); i++)
			{
				if (ds < m_laneSections[i].m_s)
					return &m_laneSections[i];
			}

			if (ds <= m_length)
				return &m_laneSections.back();
		}

		return nullptr;
	}

	void OpenDrive::Road::evaluate(double ds, double& x, double& y, double& h)
	{
		OpenDrive::Geometry* geometry = getGeometryByS(ds);
		if (geometry)
		{
			geometry->evaluate(ds, x, y, h);
		}
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
			parseRoadLink(road, roadNode);
			parseLanes(road, roadNode);

			m_roads.emplace_back(road);
		}

		refreshDebugDraw();
	}

	void OpenDrive::parseGeometry(Road& road, pugi::xml_node roadNode)
	{
		pugi::xml_node planViewNode = roadNode.child("planView");
		if (planViewNode)
		{
			for (pugi::xml_node geometryNode = planViewNode.child("geometry"); geometryNode; geometryNode = geometryNode.next_sibling("geometry"))
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
					double a = typeNode.attribute("a").as_double();
					double b = typeNode.attribute("b").as_double();
					double c = typeNode.attribute("c").as_double();
					double d = typeNode.attribute("d").as_double();
					road.m_geometries.push_back(EchoNew(Poly3(s, x, y, hdg, length, a, b, c, d)));
				}
				else if (StringUtil::Equal(typeNode.name(), "paramPoly3"))
				{
					double aU = typeNode.attribute("aU").as_double();
					double bU = typeNode.attribute("bU").as_double();
					double cU = typeNode.attribute("cU").as_double();
					double dU = typeNode.attribute("dU").as_double();
					double aV = typeNode.attribute("aV").as_double();
					double bV = typeNode.attribute("bV").as_double();
					double cV = typeNode.attribute("cV").as_double();
					double dV = typeNode.attribute("dV").as_double();

					ParamPoly3::RangeType type = ParamPoly3::RangeType::Normalized;
					if (strcmp(typeNode.attribute("pRange").as_string(), "arcLength") == 0)
						type = ParamPoly3::RangeType::ArcLength;

					road.m_geometries.push_back(EchoNew(ParamPoly3(s, x, y, hdg, length, type, aU, bU, cU, dU, aV, bV, cV, dV)));
				}
			}
		}
	}

	void OpenDrive::parseRoadLink(Road& road, pugi::xml_node roadNode)
	{
		auto setLinkData = [](RoadLink& roadLink, pugi::xml_node xmlNode)
		{
			if (xmlNode)
			{
				roadLink.m_elementId = xmlNode.attribute("elementId").as_int(-1);

				String elementType = xmlNode.attribute("elementType").as_string();
				String contactPointType = xmlNode.attribute("contactPoint").as_string();

				if (elementType == "road")
				{
					roadLink.m_elementType = RoadLink::ElementType::Road;

					if (contactPointType == "start")
					{
						roadLink.m_contactPointType = RoadLink::ContactPointType::Start;
					}
					else if (contactPointType == "end")
					{
						roadLink.m_contactPointType = RoadLink::ContactPointType::End;
					}
					else
					{
						EchoLogError("Opendrive : Unknown road link contact point type.")
					}
				}
				else if (elementType == "junction")
				{
					roadLink.m_elementType = RoadLink::ElementType::Junction;
					roadLink.m_contactPointType = RoadLink::ContactPointType::None;
				}
				else
				{
					EchoLogError("Opendrive : Unknown road link elment type.")
				}
			}
		};

		pugi::xml_node linkNode = roadNode.child("link");
		if (linkNode)
		{
			setLinkData(road.m_predecessor, linkNode.child("predecessor"));
			setLinkData(road.m_successor, linkNode.child("successor"));
		}
	}

	void OpenDrive::parseLaneLink(Lane& lane, pugi::xml_node laneNode)
	{
		auto setLinkData = [](LaneLink& laneLink, pugi::xml_node xmlNode)
		{
			if (xmlNode)
			{
				laneLink.m_id = xmlNode.attribute("id").as_int(0);
			}
		};

		pugi::xml_node linkNode = laneNode.child("link");
		if (linkNode)
		{
			setLinkData(lane.m_predecessor, linkNode.child("predecessor"));
			setLinkData(lane.m_successor, linkNode.child("successor"));
		}
	}

	void OpenDrive::parseLanes(Road& road, pugi::xml_node roadNode)
	{
		auto mappingLaneType = [](const char* name) -> LaneType
		{
			if		(stricmp(name, "shoulder") == 0)		return LaneType::Shoulder;
			else if (stricmp(name, "border") == 0)			return LaneType::Border;
			else if (stricmp(name, "driving") == 0)			return LaneType::Driving;
			else if (stricmp(name, "stop") == 0)			return LaneType::Stop;
			else if (stricmp(name, "restricted") == 0)		return LaneType::Restricted;
			else if (stricmp(name, "parking") == 0)			return LaneType::Parking;
			else if (stricmp(name, "median") == 0)			return LaneType::Median;
			else if (stricmp(name, "biking") == 0)			return LaneType::Biking;
			else if (stricmp(name, "sidewalk") == 0)		return LaneType::Sidewalk;
			else if (stricmp(name, "curb") == 0)			return LaneType::Curb;
			else if (stricmp(name, "exit") == 0)			return LaneType::Exit;
			else if (stricmp(name, "entry") == 0)			return LaneType::Entry;
			else if (stricmp(name, "onramp") == 0)			return LaneType::OnRamp;
			else if (stricmp(name, "offramp") == 0)			return LaneType::OffRamp;
			else if (stricmp(name, "connectingramp") == 0)	return LaneType::ConnectingRamp;
			else											return LaneType::None;
		};

		auto mappingRoadMarkType = [](const char* name) -> LaneRoadMark::MarkType
		{
			if		(stricmp(name, "solid") == 0)			return LaneRoadMark::MarkType::Solid;
			else if (stricmp(name, "broken") == 0)			return LaneRoadMark::MarkType::Broken;
			else if (stricmp(name, "solid solid") == 0)		return LaneRoadMark::MarkType::SolidSolid;
			else if (stricmp(name, "solid broken") == 0)	return LaneRoadMark::MarkType::SolidBroken;
			else if (stricmp(name, "broken solid") == 0)	return LaneRoadMark::MarkType::BrokenSolid;
			else if (stricmp(name, "broken broken") == 0)	return LaneRoadMark::MarkType::BrokenBroken;
			else if (stricmp(name, "botts dots") == 0)		return LaneRoadMark::MarkType::BottsDots;
			else if (stricmp(name, "grass") == 0)			return LaneRoadMark::MarkType::Grass;
			else if (stricmp(name, "curb") == 0)			return LaneRoadMark::MarkType::Curb;
			else											return LaneRoadMark::MarkType::None;
		};

		auto mappingRoadMarkWeight = [](const char* name) -> LaneRoadMark::MarkWeight
		{
			if (stricmp(name, "bold") == 0)			return LaneRoadMark::MarkWeight::Standard;
			else									return LaneRoadMark::MarkWeight::Bold;
		};

		auto mappingRoadMarkColor = [](const char* name) -> LaneRoadMark::MarkColor
		{
			if (stricmp(name, "blue") == 0)				return LaneRoadMark::MarkColor::Blue;
			else if (stricmp(name, "green") == 0)		return LaneRoadMark::MarkColor::Green;
			else if (stricmp(name, "red") == 0)			return LaneRoadMark::MarkColor::Red;
			else if (stricmp(name, "white") == 0)		return LaneRoadMark::MarkColor::White;
			else if (stricmp(name, "yellow") == 0)		return LaneRoadMark::MarkColor::Yellow;
			else										return LaneRoadMark::MarkColor::Standard;
		};

		auto mappingRoadMarkLaneChange = [](const char* name) -> LaneRoadMark::MarkLaneChange
		{
			if		(stricmp(name, "increase") == 0)	return LaneRoadMark::MarkLaneChange::Increase;
			else if (stricmp(name, "decrease") == 0)	return LaneRoadMark::MarkLaneChange::Decrease;
			else if (stricmp(name, "both") == 0)		return LaneRoadMark::MarkLaneChange::Both;
			else										return LaneRoadMark::MarkLaneChange::None;
		};

		auto parseLane = [&](LaneArray& lanes, pugi::xml_node laneParentNode)
		{
			for (pugi::xml_node laneNode = laneParentNode.child("lane"); laneNode; laneNode = laneNode.next_sibling("lane"))
			{
				Lane lane;
				lane.m_id = laneNode.attribute("id").as_int();
				lane.m_type = mappingLaneType(laneNode.attribute("type").as_string());
				lane.m_level = laneNode.attribute("level").as_int();

				parseLaneLink(lane, laneNode);

				for (pugi::xml_node widthNode = laneNode.child("width"); widthNode; widthNode = widthNode.next_sibling("width"))
				{
					double offset = widthNode.attribute("sOffset").as_double();
					double		a = widthNode.attribute("a").as_double();
					double		b = widthNode.attribute("b").as_double();
					double		c = widthNode.attribute("c").as_double();
					double		d = widthNode.attribute("d").as_double();

					lane.m_widthes.emplace_back(LaneWidth(offset, a, b, c, d));
				}

				for (pugi::xml_node roadMarkNode = laneNode.child("roadMark"); roadMarkNode; roadMarkNode = roadMarkNode.next_sibling("roadMark"))
				{
					LaneRoadMark roadMark;
					roadMark.m_offset = roadMarkNode.attribute("sOffset").as_double();
					roadMark.m_width = roadMarkNode.attribute("width").as_double();
					roadMark.m_height = roadMarkNode.attribute("height").as_double();
					roadMark.m_type = mappingRoadMarkType(roadMarkNode.attribute("type").as_string());
					roadMark.m_weight = mappingRoadMarkWeight(roadMarkNode.attribute("weight").as_string());
					roadMark.m_color = mappingRoadMarkColor(roadMarkNode.attribute("color").as_string());
					roadMark.m_laneChange = mappingRoadMarkLaneChange(roadMarkNode.attribute("laneChange").as_string());
					roadMark.m_material = LaneRoadMark::MarkMaterial::Standard;

					// Sub type

					lane.m_roadMarks.emplace_back(roadMark);
				}

				lanes.emplace_back(lane);
			}
		};

		auto addLaneSection = [&](LaneSection& laneSection)
		{
			if (!road.m_laneSections.empty())
			{
				LaneSection& preLaneSection = road.m_laneSections.back();
				preLaneSection.m_length = laneSection.m_s - preLaneSection.m_s;
			}

			laneSection.m_length = road.m_length - laneSection.m_s;

			road.m_laneSections.emplace_back(laneSection);
		};

		pugi::xml_node lanesNode = roadNode.child("lanes");
		if (lanesNode)
		{
			for (pugi::xml_node laneSectionNode = lanesNode.child("laneSection"); laneSectionNode; laneSectionNode = laneSectionNode.next_sibling("laneSection"))
			{
				LaneSection laneSection;
				laneSection.m_s = laneSectionNode.attribute("s").as_double();

				parseLane(laneSection.m_lanes, laneSectionNode.child("left"));
				parseLane(laneSection.m_lanes, laneSectionNode.child("center"));
				parseLane(laneSection.m_lanes, laneSectionNode.child("right"));

				addLaneSection(laneSection);
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