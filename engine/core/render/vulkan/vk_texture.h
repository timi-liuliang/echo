#pragma once

#include "base/texture.h"
#include "base/texture_render.h"

namespace Echo
{
    class VKTexture2D : public Texture
    {
    public:
        virtual ~VKTexture2D();
    };

    class VKTextureRender : public TextureRender
    {
    public:
        virtual ~VKTextureRender();
    };
}
