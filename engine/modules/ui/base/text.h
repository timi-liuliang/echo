#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/interface/mesh/Mesh.h"
#include "engine/core/render/interface/Material.h"
#include "engine/core/render/interface/Renderable.h"
#include "../render/vertex_format.h"

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
        const String getText() { return StringUtil::WCS2MBS(m_text); }
        
        // font res path
        void setFont(const ResourcePath& path);
        const ResourcePath& getFont() { return m_fontRes; }

		// font size
		void setFontSize(i32 fontSize);
		i32 getFontSize() const { return m_fontSize; }
        
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
        virtual void update_self() override;
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawables data
        void buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        WString                 m_text;
        ResourcePath            m_fontRes = ResourcePath("", ".ttf");
		i32						m_fontSize = 24;
        Mesh*                   m_mesh;            // Geometry Data for render
        MaterialPtr             m_material;        // Material Instance
        Renderable*             m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width;
        i32                     m_height;
    };
}
