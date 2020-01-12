#include "tilemap.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/font/font_library.h"

namespace Echo
{
    TileMap::TileMap()
    : Node()
    {
    }
    
    TileMap::~TileMap()
    {
    }
    
    void TileMap::bindMethods()
    {
        CLASS_BIND_METHOD(TileMap, getTileShape, DEF_METHOD("getTileShape"));
        CLASS_BIND_METHOD(TileMap, setTileShape, DEF_METHOD("setTileShape"));

        CLASS_REGISTER_PROPERTY(TileMap, "TileShape", Variant::Type::StringOption, "getTileShape", "setTileShape");
    }

    void TileMap::setTileShape(const StringOption& option)
    {
        m_tileShape.setValue(option.getValue());
    }
}
