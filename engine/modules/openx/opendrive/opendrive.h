#pragma once

#include "engine/core/scene/node.h"
#include "thirdparty/pugixml/pugixml.hpp"

#define PARAMPOLY3_STEPS 100

namespace Echo
{
	class OpenDriveDebugDraw;

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

			// Heading
			virtual void getHeading(double& x, double& y);

			// Hdg
			virtual double getHdg() const;
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
			double m_curvatureStart = 0.0;
			double m_curvatureEnd = 0.0;
			double m_cdot = 0.0;
			double m_x0 = 0.0;
			double m_y0 = 0.0;
			double m_h0 = 0.0;
			double m_s0 = 0.0;

			OpenDrive::Line*  m_line = nullptr;
			OpenDrive::Arc*   m_arc = nullptr;

			Spiral(double s, double x, double y, double hdg, double length, double curvatureStart, double curvatureEnd);
			virtual ~Spiral();

			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;

			// Heading
			virtual void getHeading(double& x, double& y);

			// Start position
			virtual void getStart(double& x, double& y);
		};

		struct Polynomial
		{
			double m_a = 0.0;
			double m_b = 0.0;
			double m_c = 0.0;
			double m_d = 0.0;
			double m_scale = 1.0;

			Polynomial() {}
			Polynomial(double a, double b, double c, double d, double scale);

			// Set
			void set(double a, double b, double c, double d, double scale = 1.0);

			// Evaluate
			double evaluate(double p);
			double evaluatePrim(double p);
			double evaluatePrimPrim(double p);
		};

		// Poly3
		struct Poly3 : public Geometry
		{
			Polynomial	m_poly3;
			double		m_uMax = 0.0;

			Poly3(double s, double x, double y, double hdg, double length, double a, double b, double c, double d);

			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;

		private:
			// Evaluate local
			void evaluateDsLocal(double ds, double& u, double& v);
		};

		// ParamPoly3
		struct ParamPoly3 : public Geometry
		{
			enum RangeType
			{
				Unknown,
				Normalized,
				ArcLength,
			};

			Polynomial	m_poly3U;
			Polynomial	m_poly3V;
			double		m_s2pMap[PARAMPOLY3_STEPS + 1][2];

			ParamPoly3( double s, double x, double y, double hdg, double length, RangeType type,
						double aU, double bU, double cU, double dU,
						double aV, double bV, double cV, double dV)
				: Geometry(s, x, y, hdg, length, Geometry::ParamPoly3)
			{
				m_poly3U.set(aU, bU, cU, dU, type == RangeType::Normalized ? 1.0 / length : 1.0);
				m_poly3V.set(aV, bV, cV, dV, type == RangeType::Normalized ? 1.0 / length : 1.0);

				calcs2pMap(type);
			}

			// Evaluate
			virtual void evaluate(double ds, double& x, double& y, double& h) override;

			// s2p
			double s2p(double s);

		private:
			// Calculate s2p map
			void calcs2pMap(RangeType type);
		};

		// Road link
		struct RoadLink
		{
			enum class ElementType
			{
				Unknow,
				Road,
				Junction,
			}						m_elementType = ElementType::Unknow;
			i32						m_elementId = -1;
			enum class ContactPointType
			{
				Unknow,
				Start,
				End,
				None,
			}						m_contactPointType = ContactPointType::Unknow;
		};

		struct Elevation
		{
			Polynomial	m_poly3;
			double		m_s = 0.0;
			double		m_length = 0.0;

			Elevation(double s, double a, double b, double c, double d)
				: m_s(s)
			{
				m_poly3.set(a, b, c, d);
			}
		};
		typedef vector<Elevation>::type ElevationArray;

		enum class LaneType
		{
			None    		= 1 << 0,	// Describes the space on the outermost edge of the road and does not have actual content Its only purpose is 
										// for applications to register that OpenDRIVE is still present in case the (human) driver leaves the road.
			Shoulder		= 1 << 1,	// Describes a soft border at the edge of the road.
			Border			= 1 << 2,	// Describes a hard border at the edge of the road. It has the same height as the drivable lane.
			Driving			= 1 << 3,	// Describes a "normal" drivable road that is not one of the other types.
			Stop			= 1 << 4,	// Hard shoulder on motorways for emergency stops
			Restricted		= 1 << 5,	// Describes a lane on which cars should not drive. The lane has the same height as drivable lanes. Typically, 
										// the lane is separated with lines and often contains dotted lines as well.
			Parking			= 1 << 6,	// Describes a lane with parking spaces.
			Median			= 1 << 7,	// Describes a lane that sits between driving lanes that lead in opposite directions. It is typically used to 
										// separate traffic in towns on large roads.
			Biking			= 1 << 8,	// Describes a lane that is reserved for cyclists.
			Sidewalk		= 1 << 9,	// Describes a lane on which pedestrians can walk.
			Curb			= 1 << 10,	// Describes curb stones. Curb stones have a different height than the adjacent drivable lanes.
			Exit			= 1 << 11,	// Describes a lane that is used for sections that are parallel to the main road. It is mainly used for deceleration lanes.
			Entry			= 1 << 12,	// Describes a lane that is used for sections that are parallel to the main road. It is mainly used for deceleration lanes.
			OnRamp			= 1 << 13,	// A ramp leading to a motorway from rural or urban roads.
			OffRamp			= 1 << 14,	// A ramp leading away from a motorway and onto rural urban roads.
			ConnectingRamp	= 1 << 15,	// A ramp that connects two motorways, for example, motorway junctions.
			
			AnyDriving		= Driving | Entry | Exit | OffRamp | OnRamp | Parking,
			AnyRoad			= AnyDriving | Restricted | Stop,
			Any				= 0xFFFFFFF,
		};

		struct LaneWidth
		{
			double		m_offset = 0.0;
			Polynomial	m_poly3;

			LaneWidth(double offset, double a, double b, double c, double d)
				: m_offset(offset)
			{
				m_poly3.set(a, b, c, d);
			}
		};
		typedef vector<LaneWidth>::type LaneWidthArray;

		struct LaneRoadMark
		{
			double		m_offset = 0.0;
			double		m_width = 0.0;
			double		m_height = 0.0;

			enum class MarkType
			{
				None = 1,
				Solid,
				Broken,
				SolidSolid,
				SolidBroken,
				BrokenSolid,
				BrokenBroken,
				BottsDots,
				Grass,
				Curb,
			}			m_type = MarkType::None;

			enum class MarkWeight
			{
				Standard,
				Bold,
			}			m_weight = MarkWeight::Standard;

			enum class MarkColor
			{
				Standard,		// Equivalent to white
				Blue,
				Green,
				Red,
				White,
				Yellow,
			}			m_color = MarkColor::Standard;

			enum class MarkMaterial
			{
				Standard,
			}			m_material = MarkMaterial::Standard;

			enum class MarkLaneChange
			{
				Increase,
				Decrease,
				Both,
				None,
			}			m_laneChange = MarkLaneChange::None;
		};
		typedef vector<LaneRoadMark>::type LaneRoadMarkArray;

		struct LaneLink
		{
			i32		m_id = 0;
		};

		struct Lane
		{
			i32					m_id = -1;
			i32					m_globalId = -1;					// Unique id for osi
			LaneType			m_type = LaneType::None;	
			i32					m_level = 0;						// Boolean, ture means keep lane on level
			LaneLink			m_predecessor;
			LaneLink			m_successor;
			LaneWidthArray		m_widthes;
			LaneRoadMarkArray	m_roadMarks;

			// Get width
			LaneWidth* getWidth(double s);
		};
		typedef vector<Lane>::type LaneArray;

		// Each lane section contains a fixed number of lanes. Every time the number of lanes changes, a new lane section is required
		struct LaneSection
		{
			double			m_s=0.0;				// s-coordinate of start position
			bool			m_singleSide = false;
			double			m_length = 0.0;
			LaneArray		m_lanes;

			// Length
			double getLength() const { return m_length; }

			// Get lane
			Lane* getLaneById(i32 laneId);

			// Get lane inner|center|outer line info
			double getLaneWidth(double s, int laneId);
			double getLaneInnerOffset(double s, int laneId);
			double getLaneCenterOffset(double s, int laneId);
			double getLaneOuterOffset(double s, int laneId);

			// Get lane heading
			double getLaneInnerOffsetHeading(double s, int laneId);
			double getLaneCenterOffsetHeading(double s, int laneId);
			double getLaneOuterOffsetHeading(double s, int laneId);
		};
		typedef vector<LaneSection>::type LaneSectionArray;

		// Road
		struct Road
		{
			String				m_name;
			double				m_length = 0;
			i32					m_id = -1;
			i32					m_junction = -1;
			GeometryArray		m_geometries;
			RoadLink			m_predecessor;
			RoadLink			m_successor;
			ElevationArray		m_elevationProfile;
			ElevationArray		m_superElevationProfile;
			LaneSectionArray	m_laneSections;

			// Get by s
			Geometry* getGeometryByS(double ds);
			LaneSection* getLaneSectionByS(double ds);

			// Evaluate reference line
			void evaluate(double s, double& x, double& y, double& h);
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
		void parseRoadLink(Road& road, pugi::xml_node roadNode);
		void parseLanes(Road& road, pugi::xml_node roadNode);
		void parseLaneLink(Lane& lane, pugi::xml_node laneNode);

		// Refresh debug draw
		void refreshDebugDraw();

		// Update
		virtual void updateInternal(float elapsedTime) override;

	protected:
		ResourcePath		m_xodrRes = ResourcePath("", ".xodr");
		vector<Road>::type	m_roads;
		OpenDriveDebugDraw*	m_debugDraw = nullptr;
		DebugDrawOption		m_debugDrawOption = DebugDrawOption::Editor;
	};
}
