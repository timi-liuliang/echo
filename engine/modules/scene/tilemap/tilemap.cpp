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
		CLASS_BIND_METHOD(TileMap, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(TileMap, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(TileMap, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(TileMap, setHeight, DEF_METHOD("setHeight"));
		CLASS_BIND_METHOD(TileMap, getTileSize, DEF_METHOD("getTileSize"));
		CLASS_BIND_METHOD(TileMap, setTileSize, DEF_METHOD("setTileSize"));
		CLASS_BIND_METHOD(TileMap, isFlipX, DEF_METHOD("isFlipX"));
		CLASS_BIND_METHOD(TileMap, setFlipX, DEF_METHOD("setFlipX"));
		CLASS_BIND_METHOD(TileMap, isFlipY, DEF_METHOD("isFlipY"));
		CLASS_BIND_METHOD(TileMap, setFlipY, DEF_METHOD("setFlipY"));

        CLASS_REGISTER_PROPERTY(TileMap, "TileShape", Variant::Type::StringOption, "getTileShape", "setTileShape");
        CLASS_REGISTER_PROPERTY(TileMap, "Width", Variant::Type::Int, "getWidth", "setWidth");
        CLASS_REGISTER_PROPERTY(TileMap, "Height", Variant::Type::Int, "getHeight", "setHeight");
        CLASS_REGISTER_PROPERTY(TileMap, "TileSize", Variant::Type::Vector2, "getTileSize", "setTileSize");
        CLASS_REGISTER_PROPERTY(TileMap, "FlipX", Variant::Type::Bool, "isFlipX", "setFlipX");
        CLASS_REGISTER_PROPERTY(TileMap, "FlipY", Variant::Type::Bool, "isFlipY", "setFlipY");
    }

    void TileMap::setTileShape(const StringOption& option)
    {
        m_tileShape.setValue(option.getValue());
    }

    Vector3 TileMap::flip(const Vector3& pos)
    {
        Vector3 result = pos;
        if (m_isFlipX)
            result.x = -result.x;

        if (m_isFlipY)
            result.y = -result.y;

        return result;
    }
}
