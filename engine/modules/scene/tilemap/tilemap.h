#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
    class TileMap : public Node
    {
        ECHO_CLASS(TileMap, Node)
        
    public:
        TileMap();
        virtual ~TileMap();
        
        // tile shape
        const StringOption& getTileShape() const { return m_tileShape; }
        void setTileShape(const StringOption& option);

		// width
		i32 getWidth() const { return m_width; }
        void setWidth(i32 width) { m_width = width; }

		// height
		i32 getHeight() const { return m_height; }
        void setHeight(i32 height) { m_height = height; }

		// grid size
		const Vector2& getTileSize() const { return m_tileSize; }
        void setTileSize(const Vector2& tileSize) { m_tileSize = tileSize; }

		// flip x
		bool isFlipX() const { return m_isFlipX; }
		void setFlipX(bool isFlipX) { m_isFlipX = isFlipX; }

		// flip y
		bool isFlipY() const { return m_isFlipY; }
		void setFlipY(bool isFlipY) { m_isFlipY = isFlipY; }

        // flip
        Vector3 flip(const Vector3& pos);
        
    private:
        StringOption        m_tileShape = StringOption("Square", { "Square"/*,"Isometric","Hexagon"*/ });
		i32                 m_width = 8;
		i32                 m_height = 8;
        Vector2             m_tileSize = Vector2(60.f, 60.f);
        bool                m_isFlipX = false;
        bool                m_isFlipY = false;
    };
}
