#pragma once

#include "engine/core/scene/node.h"
#include "thirdparty/pugixml/pugixml.hpp"

namespace Echo
{
	// https://www.asam.net/standards/detail/opendrive/
	class OpenDrive : public Node
	{
		ECHO_CLASS(OpenDrive, Node)

	public:
		// Geometry
		struct Geometry
		{
			enum Type
			{
				Unknow,
				Line,
				Arc,
				Spiral,
				Poly3,
				ParamPoly3,
			}	   m_type = Unknow;
			double m_s = 0.0;
			double m_x = 0.0;
			double m_y = 0.0;
			double m_hdg = 0.0;
			double m_length = 0.0;

			Geometry(double s, double x, double y, double hdg, double length, Type type)
				: m_s(s)
				, m_x(x)
				, m_y(y)
				, m_hdg(hdg)
				, m_length(length)
				, m_type(type)
			{}
		};
		typedef vector<Geometry*>::type GeometryArray;

		// Line
		struct Line : public Geometry
		{
			Line(double s, double x, double y, double hdg, double length)
				: Geometry(s, x, y, hdg, length, Geometry::Line)
			{}
		};

		// Line
		struct Arc : public Geometry
		{
			double	m_curvature = 0.0;

			Arc(double s, double x, double y, double hdg, double length, double curvature)
				: Geometry(s, x, y, hdg, length, Geometry::Arc)
				, m_curvature(curvature)
			{}
		};

		// Spiral
		struct Spiral : public Geometry
		{

		};

		// Poly3
		struct Poly3 : public Geometry
		{

		};

		// ParamPoly3
		struct ParamPoly3 : public Geometry
		{

		};

		// Road
		struct Road
		{
			String			m_name;
			double			m_length = 0;
			i32				m_id = -1;
			i32				m_junction = -1;
			GeometryArray	m_geometries;
		};

	public:
		OpenDrive();
		virtual ~OpenDrive();

		// Xodr file
		void setXodrRes(const ResourcePath& path);
		const ResourcePath& getXodrRes() { return m_xodrRes; }

	private:
		// Parse
		void parseXodr(const String& content);
		void parseGeometry(Road& road, pugi::xml_node roadNode);

		// Update
		virtual void updateInternal() override;

	public:
		ResourcePath		m_xodrRes = ResourcePath("", ".xodr");
		vector<Road>::type	m_roads;
	};
}
