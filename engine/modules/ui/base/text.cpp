#include "text.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/font/font_library.h"

namespace Echo
{
    UiText::UiText()
    : UiRender()
	, m_mesh(nullptr)
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
    
    void UiText::setText(const String& text)
    {
        m_text = StringUtil::MBS2WCS(text);
		updateMeshBuffer();
    }
    
    void UiText::setFont(const ResourcePath& path)
    {
        if (m_fontRes.setPath(path.getPath()))
        {
			updateMeshBuffer();
        }
    }

	void UiText::setFontSize(i32 fontSize)
	{ 
		m_fontSize = fontSize;
		if (m_fontSize > 0)
		{
			updateMeshBuffer();
		}
	}
    
    void UiText::setWidth(i32 width)
    {
        if (m_width != width)
        {
            m_width = width;
            
			updateMeshBuffer();
        }
    }
    
    void UiText::setHeight(i32 height)
    {
        if (m_height != height)
        {
            m_height = height;
            
            updateMeshBuffer();
        }
    }
    
    void UiText::buildRenderable()
    {
        if (!m_text.empty() && !m_fontRes.isEmpty())
        {
            clearRenderable();
            
            StringArray macros = {"ALPHA_ADJUST"};
            m_shader = ShaderProgram::getDefault2D(macros);
            
            // material
            m_material = EchoNew(Material(StringUtil::Format("UiTextMaterial_%d", getId())));
            m_material->setShaderPath(m_shader->getPath());
            
            // mesh
            Ui::VertexArray vertices;
            Ui::IndiceArray indices;
            buildMeshData(vertices, indices);
            
            MeshVertexFormat define;
            define.m_isUseUV = true;
            
            m_mesh = Mesh::create(true, true);
            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
            
            m_renderable = Renderable::create(m_mesh, m_material, this);
        }
    }
    
    void UiText::update_self()
    {
        if (isNeedRender())
        {
            if (m_renderable)
            {
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
            for(wchar_t glyphCode : m_text)
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
					oVertices.emplace_back(Vector3(left, top, 0.f), Vector2(uvLeft, uvTop));
					oVertices.emplace_back(Vector3(left, bottom, 0.f), Vector2(uvLeft, uvBottom));
					oVertices.emplace_back(Vector3(right, bottom, 0.f), Vector2(uvRight, uvBottom));
					oVertices.emplace_back(Vector3(right, top, 0.f), Vector2(uvRight, uvTop));

					// indices
					oIndices.emplace_back(vertBase + 0);
					oIndices.emplace_back(vertBase + 1);
					oIndices.emplace_back(vertBase + 2);
					oIndices.emplace_back(vertBase + 0);
					oIndices.emplace_back(vertBase + 2);
					oIndices.emplace_back(vertBase + 3);

                    m_material->getUniform("BaseColor")->setTexture(fontGlyph->m_texture->getTexture());
                }

				m_width += m_fontSize;
            }
        }
          
        // Calculate aabb
        m_localAABB.reset();
        for (Ui::VertexFormat& vert : oVertices)
            m_localAABB.addPoint(vert.m_position);
    }
    
    void UiText::updateMeshBuffer()
    {
		if (!m_mesh)
		{
			buildRenderable();
		}

		if (m_mesh)
		{
			Ui::VertexArray    vertices;
			Ui::IndiceArray    indices;
			buildMeshData(vertices, indices);

			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
		}
    }
    
    void UiText::clear()
    {
        clearRenderable();
    }
    
    void UiText::clearRenderable()
    {
        EchoSafeRelease(m_renderable);
        m_mesh.reset();
    }
}
