#include "text.h"
#include "material.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "interface/Renderer.h"
#include "interface/ShaderProgram.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/font/font_library.h"

namespace Echo
{
    UiText::UiText()
    : m_mesh(nullptr)
    , m_material(nullptr)
    , m_renderable(nullptr)
    , m_width(0)
    , m_height(0)
    {
    }
    
    UiText::~UiText()
    {
        clear();
    }
    
    void UiText::bindMethods()
    {
        CLASS_BIND_METHOD(UiText, getText,          DEF_METHOD("getText"));
        CLASS_BIND_METHOD(UiText, setText,          DEF_METHOD("setText"));
        CLASS_BIND_METHOD(UiText, getFont,          DEF_METHOD("getFont"));
        CLASS_BIND_METHOD(UiText, setFont,          DEF_METHOD("setFont"));
		CLASS_BIND_METHOD(UiText, getFontSize,		DEF_METHOD("getFontSize"));
		CLASS_BIND_METHOD(UiText, setFontSize,		DEF_METHOD("setFontSize"));
        CLASS_BIND_METHOD(UiText, getWidth,         DEF_METHOD("getWidth"));
        CLASS_BIND_METHOD(UiText, setWidth,         DEF_METHOD("setWidth"));
        CLASS_BIND_METHOD(UiText, getHeight,        DEF_METHOD("getHeight"));
        CLASS_BIND_METHOD(UiText, setHeight,        DEF_METHOD("setHeight"));
        
        CLASS_REGISTER_PROPERTY(UiText, "Width", Variant::Type::Int, "getWidth", "setWidth");
        CLASS_REGISTER_PROPERTY(UiText, "Height", Variant::Type::Int, "getHeight", "setHeight");
        CLASS_REGISTER_PROPERTY(UiText, "Text", Variant::Type::String, "getText", "setText");
        CLASS_REGISTER_PROPERTY(UiText, "Font", Variant::Type::ResourcePath, "getFont", "setFont");
		CLASS_REGISTER_PROPERTY(UiText, "FontSize", Variant::Type::Int, "getFontSize", "setFontSize");
    }
    
    // set texture res path
    void UiText::setText(const String& text)
    {
        m_text = text;
        buildRenderable();
    }
    
    void UiText::setFont(const ResourcePath& path)
    {
        if (m_fontRes.setPath(path.getPath()))
        {
            buildRenderable();
        }
    }

	void UiText::setFontSize(i32 fontSize)
	{ 
		m_fontSize = fontSize;
		if (m_fontSize > 0)
		{
			buildRenderable();
		}
	}
    
    void UiText::setWidth(i32 width)
    {
        if (m_width != width)
        {
            m_width = width;
            
            buildRenderable();
        }
    }
    
    // width
    void UiText::setHeight(i32 height)
    {
        if (m_height != height)
        {
            m_height = height;
            
            buildRenderable();
        }
    }
    
    void UiText::buildRenderable()
    {
        if (!m_text.empty())
        {
            clearRenderable();
            
            // material
            m_material = ECHO_CREATE_RES(Material);
            m_material->setShaderContent("echo_text_default_shader", UiMaterial::getDefault());
            m_material->setRenderStage("Transparent");
            
            // mesh
            Ui::VertexArray vertices;
            Ui::IndiceArray indices;
            buildMeshData(vertices, indices);
            
            MeshVertexFormat define;
            define.m_isUseUV = true;
            
            m_mesh = Mesh::create(true, true);
            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);
            
            m_renderable = Renderable::create(m_mesh, m_material, this);
        }
    }
    
    // update per frame
    void UiText::update_self()
    {
        if (isNeedRender())
        {
            if (m_renderable)
            {
                m_matWVP = getWorldMatrix() * NodeTree::instance()->get2dCamera()->getViewProjMatrix();
                m_renderable->submitToRenderQueue();
            }
        }
    }
    
    void UiText::buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices)
    {
        if(!m_text.empty() && !m_fontRes.isEmpty())
        {
			m_width = 0;
            m_height = m_fontSize;
            WString wText = StringUtil::MBS2WCS( m_text);
            for(wchar_t glyphCode : wText)
            {
                FontGlyph* fontGlyph = FontLibrary::instance()->getFontGlyph( glyphCode, m_fontRes, m_fontSize);
                if(fontGlyph)
                {
					float left = m_width;
					float right = left + m_fontSize;
					float top = m_height;
					float bottom = 0;

					Vector4 uv = fontGlyph->getUV();
					float uvLeft = uv.x;
					float uvTop = uv.y;
					float uvRight = uv.x + uv.z;
					float uvBottom = uv.y + uv.w;

					// vertices
					Word vertBase = oVertices.size();
					oVertices.push_back(Ui::VertexFormat(Vector3(left, top, 0.f), Vector2(uvLeft, uvTop)));
					oVertices.push_back(Ui::VertexFormat(Vector3(left, bottom, 0.f), Vector2(uvLeft, uvBottom)));
					oVertices.push_back(Ui::VertexFormat(Vector3(right, bottom, 0.f), Vector2(uvRight, uvBottom)));
					oVertices.push_back(Ui::VertexFormat(Vector3(right, top, 0.f), Vector2(uvRight, uvTop)));

					// indices
					oIndices.push_back(vertBase + 0);
					oIndices.push_back(vertBase + 1);
					oIndices.push_back(vertBase + 2);
					oIndices.push_back(vertBase + 0);
					oIndices.push_back(vertBase + 2);
					oIndices.push_back(vertBase + 3);

                    m_material->setTexture("u_BaseColorSampler", fontGlyph->m_texture->getTexture());
                }

				m_width += m_fontSize;
            }
        }
          
        // calc aabb
        m_localAABB.reset();
        for (Ui::VertexFormat& vert : oVertices)
            m_localAABB.addPoint(vert.m_position);
    }
    
    // update vertex buffer
    void UiText::updateMeshBuffer()
    {
        Ui::VertexArray    vertices;
        Ui::IndiceArray    indices;
        buildMeshData(vertices, indices);
        
        MeshVertexFormat define;
        define.m_isUseUV = true;
        
        m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
        m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);
    }
    
    void UiText::clear()
    {
        clearRenderable();
    }
    
    void UiText::clearRenderable()
    {
        EchoSafeRelease(m_renderable);
        EchoSafeRelease(m_material);
        EchoSafeRelease(m_mesh);
    }
}
