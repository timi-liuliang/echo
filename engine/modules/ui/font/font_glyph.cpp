#include "font_glyph.h"

namespace Echo
{
    FontGlyph::FontGlyph()
    {
        
    }
    
    FontGlyph::~FontGlyph()
    {
        
    }

    float FontGlyph::getWidth()
    {
        const FontTexture::Node& node = m_texture->getNode(m_nodeIndex);
        return node.m_rc.width;
    }

    float FontGlyph::getHeight()
    {
		const FontTexture::Node& node = m_texture->getNode(m_nodeIndex);
		return node.m_rc.height;
    }
}
