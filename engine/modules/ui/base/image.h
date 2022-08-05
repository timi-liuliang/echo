#pragma once

#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/shader/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include "../render/vertex_format.h"
#include "render.h"

namespace Echo
{
    class UiImage : public UiRender
    {
        ECHO_CLASS(UiImage, UiRender)
        
    public:
        UiImage();
        virtual ~UiImage();
        
        // Texture res path
        void setTextureRes(const ResourcePath& path);
        const ResourcePath& getTextureRes() { return m_textureRes; }

        // Tint color
        const Color& getColor() const { return m_color; }
        void setColor(const Color& color);
        
        // width
        i32 getWidth() const { return m_width; }
        void setWidth(i32 width);
        
        // width
        i32 getHeight() const { return m_height; }
        void setHeight(i32 height);

        // Anchor
        const Vector2& getAnchor() const { return m_anchor; }
        void setAnchor(const Vector2& anchor);
        
        // material
        Material* getMaterial() const { return m_material; }
        void setMaterial(Object* material);

    protected:
        // build drawable
        void buildRenderable();
        
        // update
        virtual void updateInternal(float elapsedTime) override;
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawables data
        void buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        ResourcePath            m_textureRes = ResourcePath("", ".png|.rt");
        Color                   m_color = Color::WHITE;
        MeshPtr                 m_mesh;
        MaterialPtr             m_material;
        RenderProxyPtr          m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width = 128;
        i32                     m_height = 128;
        Vector2                 m_anchor = Vector2::ZERO;
    };
}
