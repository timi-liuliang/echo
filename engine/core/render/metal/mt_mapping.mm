#include "mt_mapping.h"

namespace Echo
{
    MTLPrimitiveType MTMapping::MapPrimitiveTopology(Mesh::TopologyType type)
    {
        switch(type)
        {
            case Mesh::TT_POINTLIST:        return MTLPrimitiveTypePoint;
            case Mesh::TT_LINELIST:         return MTLPrimitiveTypeLine;
            case Mesh::TT_LINESTRIP:        return MTLPrimitiveTypeLineStrip;
            case Mesh::TT_TRIANGLELIST:     return MTLPrimitiveTypeTriangle;
            case Mesh::TT_TRIANGLESTRIP:    return MTLPrimitiveTypeTriangleStrip;
            default:                        return MTLPrimitiveTypeTriangle;
        }
    }
}
