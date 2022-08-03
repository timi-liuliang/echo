#pragma once

#include "engine/core/base/object.h"
#include <nlohmann/json.hpp>

namespace Echo
{
	class OpenLabel : public Object
	{
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
			bool					m_closed;

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
			String	m_name;
			String	m_type;
		};
		typedef vector<LabelObject>::type LabelObjectArray;

	public:
		OpenLabel();
		~OpenLabel();

		// Parse
		void parse();

		// Save
		void save();

	protected:
		// Save
		void saveObjects(nlohmann::json& parent);

	protected:
		LabelObjectArray		m_objects;
	};
}