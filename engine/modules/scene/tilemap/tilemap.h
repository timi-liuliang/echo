#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"
#include "engine/core/render/base/image/Image.h"

namespace Echo
{
    class TileMap : public Node
    {
        ECHO_CLASS(TileMap, Node)
        
    public:
        TileMap();
        virtual ~TileMap();
        
    private:
    };
}
