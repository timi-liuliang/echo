#pragma once

#include "engine/core/render/base/image/pixel_format.h"
#include "engine/modules/pcg/data/pcg_data.h"

namespace Echo
{
	class PCGImage : public PCGData
	{
	public:
		PCGImage();
		~PCGImage();

		// Type
		virtual String getType() { return "Image"; }

		// Set
		void set(i32 width, i32 height);

		// Set Value
		void setValue(i32 x, i32 y, const Color& color);

		// Width
		i32 getWidth() const { return m_width; }

		// Height
		i32 getHeight() const { return m_height; }

		// Depth
		i32 getDepth() const { return m_depth; }

		// Data
		const vector<Color>::type& getColors() const { return m_colors; }

	protected:
		i32					m_width = 0;
		i32					m_height = 0;
		i32					m_depth = 1;
		vector<Color>::type	m_colors;
	};
	typedef ResRef<PCGImage> PCGImagePtr;
}
