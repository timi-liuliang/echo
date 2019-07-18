#pragma once

#include "engine/core/render/interface/mesh/Mesh.h"
#include "mt_render_base.h"

namespace Echo
{
    class MTMapping
    {
    public:
        // Mapping Primitive Type
        static MTLPrimitiveType MapPrimitiveTopology(Mesh::TopologyType type);
    };
}
