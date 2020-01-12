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
        
    private:
        StringOption        m_tileShape = StringOption("Square", { "Square","Isometric","Hexagon" });
    };
}
