#pragma once

#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
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
        
        // texture res path
        void setTextureRes(const ResourcePath& path);
        const ResourcePath& getTextureRes() { return m_textureRes; }
        
        // width
        i32 getWidth() const { return m_width; }
        void setWidth(i32 width);
        
        // width
        i32 getHeight() const { return m_height; }
        void setHeight(i32 height);
        
        // material
        Material* getMaterial() const { return m_material; }
        void setMaterial( Object* material) { m_material = (Material*)material; }

    protected:
        // build drawable
        void buildRenderable();
        
        // update
        virtual void update_self() override;
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawables data
        void buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        ResourcePath            m_textureRes;
        MeshPtr                 m_mesh;            // Geometry Data for render
        ShaderProgramPtr        m_shader;
        MaterialPtr             m_material;
        MaterialPtr             m_materialDefault;
        RenderProxy*             m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width = 128;
        i32                     m_height = 128;
    };
}
