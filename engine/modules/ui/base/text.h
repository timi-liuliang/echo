#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/interface/mesh/Mesh.h"
#include "engine/core/render/interface/Material.h"
#include "engine/core/render/interface/Renderable.h"
#include "vertex_format.h"

namespace Echo
{
    class UiText : public Render
    {
        ECHO_CLASS(UiText, Render)
        
    public:
        UiText();
        virtual ~UiText();
        
        // texture res path
        void setText(const String& text);
        const String& getText() { return m_text; }
        
        // width
        i32 getWidth() const { return m_width; }
        void setWidth(i32 width);
        
        // width
        i32 getHeight() const { return m_height; }
        void setHeight(i32 height);
        
    protected:
        // build drawable
        void buildRenderable();
        
        // update
        virtual void update_self();
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawables data
        void buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        String                  m_text;
        Mesh*                   m_mesh;            // Geometry Data for render
        Material*               m_material;        // Material Instance
        Renderable*             m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width;
        i32                     m_height;
    };
}
