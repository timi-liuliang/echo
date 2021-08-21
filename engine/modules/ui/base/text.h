#pragma once

#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include "../render/vertex_format.h"
#include "render.h"

namespace Echo
{
    class UiText : public UiRender
    {
        ECHO_CLASS(UiText, UiRender)
        
    public:
        UiText();
        virtual ~UiText();
        
        // texture res path
        void setText(const String& text);
        const String getText() { return StringUtil::WCS2MBS(m_text); }
        
        // Font res path
        void setFont(const ResourcePath& path);
        const ResourcePath& getFont() { return m_fontRes; }

		// Font size
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
        virtual void updateInternal(float elapsedTime) override;
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawable data
        void buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        WString                 m_text;
        ResourcePath            m_fontRes = ResourcePath("", ".ttf");
		i32						m_fontSize = 24;
        MeshPtr                 m_mesh;            // Geometry Data for render
        ShaderProgramPtr        m_shader;
        MaterialPtr             m_material;        // Material Instance
        RenderProxyPtr          m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width;
        i32                     m_height;
    };
}
