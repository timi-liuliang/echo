#pragma once

#include "engine/core/scene/node.h"
#include <nlohmann/json.hpp>

namespace Echo
{
	class OpenLabelDebugDraw;

	class OpenLabel : public Node
	{
		ECHO_CLASS(OpenLabel, Node)

	public:
		struct LabelRegion
		{
			LabelRegion(){}
			virtual ~LabelRegion() {}

			// Type
			virtual String getType() { return "region"; }
		};

		struct BBox : public LabelRegion
		{
			bool m_visible = false;
		};

		struct RBBox : public LabelRegion
		{

		};

		struct Poly2d : public LabelRegion
		{
			enum Mode
			{
				MODE_POLY2D_SRF6DCC,
				MODE_POLY2D_ABSOLUTE
			}						m_mode = MODE_POLY2D_ABSOLUTE;

			vector<Vector2>::type	m_values;
			vector<bool>::type		m_visibles;
			bool					m_closed = true;

			// Type
			virtual String getType() { return "poly2d"; }
		};

		struct Cuboid2d : public LabelRegion
		{
			vector<Vector2>::type	m_values;
			vector<bool>::type		m_visibles;

			// Type
			virtual String getType() { return "cuboid2d"; }
		};

		struct LabelObject
		{
			String					m_name;
			String					m_type;
			vector<Poly2d>::type	m_poly2ds;
			vector<Cuboid2d>::type	m_cuboid2ds;
		};
		typedef vector<LabelObject>::type LabelObjectArray;

	public:
		OpenLabel();
		~OpenLabel();

		// Xodr file
		void setRes(const ResourcePath& path);
		const ResourcePath& getRes() { return m_resPath; }

		// Debug draw
		OpenLabelDebugDraw* getDebugDraw();
		void setDebugDraw(Object* debugDraw);

		// Objects
		LabelObjectArray& getObjects() { return m_objects; }

		// Reset
		void reset();

	public:
		// Parse
		void parse(const String& content);

		// Save
		void save(const char* savePath);

	protected:
		// Parse
		void parseObjects(nlohmann::json& parent);
		void parsePoly2ds(nlohmann::json& parentJson, LabelObject& object);
		void parseCuboid2ds(nlohmann::json& parentJson, LabelObject& object);

		// Save
		void saveObjects(nlohmann::json& parent);
		void savePoly2ds(nlohmann::json& parentJson, LabelObject& object);
		void saveCuboid2ds(nlohmann::json& parentJson, LabelObject& object);

	private:
		// Refresh debug draw
		void refreshDebugDraw();

		// Update
		virtual void updateInternal(float elapsedTime) override;

	protected:
		ResourcePath			m_resPath = ResourcePath("", ".json");
		bool					m_dirty = true;
		LabelObjectArray		m_objects;
		OpenLabelDebugDraw*		m_debugDraw = nullptr;
	};
}