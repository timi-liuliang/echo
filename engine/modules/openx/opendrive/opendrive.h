#pragma once

#include "engine/core/scene/node.h"
#include "thirdparty/pugixml/pugixml.hpp"
#include "opendrive_debug_draw.h"

namespace Echo
{
	// https://www.asam.net/standards/detail/opendrive/
	class OpenDrive : public Node
	{
		ECHO_CLASS(OpenDrive, Node)

	public:
		// Debug draw option
		enum DebugDrawOption
		{
			None,
			Editor,
			Game,
			All
		};

		// Geometry 
		// https://gupea.ub.gu.se/bitstream/2077/23047/1/gupea_2077_23047_1.pdf
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
			double m_s = 0.0;		// Start position(s-coordinate)
			double m_x = 0.0;		// Start position(x inertial)
			double m_y = 0.0;		// Start position(y inertial)
			double m_hdg = 0.0;		// Inital heading
			double m_length = 0.0;	// Length of the element's reference line

			Geometry(double s, double x, double y, double hdg, double length, Type type)
				: m_s(s)
				, m_x(x)
				, m_y(y)
				, m_hdg(hdg)
				, m_length(length)
				, m_type(type)
			{}

			// Evaluate
			virtual void evaluate(double sampleLength, double& x, double& y, double& h) {}

			// Length 
			double getLength() const { return m_length; }
		};
		typedef vector<Geometry*>::type GeometryArray;

		// Line
		struct Line : public Geometry
		{
			Line(double s, double x, double y, double hdg, double length)
				: Geometry(s, x, y, hdg, length, Geometry::Line)
			{}

			// Evaluate
			virtual void evaluate(double sampleLength, double& x, double& y, double& h) override;
		};

		// Line
		struct Arc : public Geometry
		{
			double	m_curvature = 0.0;

			Arc(double s, double x, double y, double hdg, double length, double curvature)
				: Geometry(s, x, y, hdg, length, Geometry::Arc)
				, m_curvature(curvature)
			{}

			// Evaluate
			virtual void evaluate(double sampleLength, double& x, double& y, double& h) override;

			// Radius
			double getRadius() const { return 1.0 / std::fabs(m_curvature); }

			// Center
			void getCenter(double& x, double& y);
		};

		// Spiral
		struct Spiral : public Geometry
		{
			double	m_curvatureStart = 0.0;
			double	m_curvatureEnd = 0.0;

			Spiral(double s, double x, double y, double hdg, double length, double curvatureStart, double curvatureEnd)
				: Geometry(s, x, y, hdg, length, Geometry::Spiral)
				, m_curvatureStart(curvatureStart)
				, m_curvatureEnd(curvatureEnd)
			{}

			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;
		};

		// Poly3
		struct Poly3 : public Geometry
		{
			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;
		};

		// ParamPoly3
		struct ParamPoly3 : public Geometry
		{
			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;
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

		// Debug draw
		StringOption getDebugDrawOption() const;
		void setDebugDrawOption(const StringOption& option);

		// Roads
		vector<Road>::type& getRoads() { return m_roads; }

		// Reset
		void reset();

	private:
		// Parse
		void parseXodr(const String& content);
		void parseGeometry(Road& road, pugi::xml_node roadNode);

		// Refresh debug draw
		void refreshDebugDraw();

		// Update
		virtual void updateInternal(float elapsedTime) override;

	protected:
		ResourcePath		m_xodrRes = ResourcePath("", ".xodr");
		vector<Road>::type	m_roads;
		OpenDriveDebugDraw	m_debugDraw;
		DebugDrawOption		m_debugDrawOption = DebugDrawOption::Editor;
	};
}
