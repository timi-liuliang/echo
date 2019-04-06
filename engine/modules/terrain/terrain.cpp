#include "terrain.h"
//#include "material.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "interface/Renderer.h"
#include "interface/ShaderProgram.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/font/font_library.h"

namespace Echo
{
    Terrain::Terrain()
    : m_mesh(nullptr)
    , m_material(nullptr)
    , m_renderable(nullptr)
    , m_width(0)
    , m_height(0)
    {
    }
    
    Terrain::~Terrain()
    {
        clear();
    }
    
    void Terrain::bindMethods()
    {
        CLASS_BIND_METHOD(Terrain, getText,          DEF_METHOD("getText"));
        CLASS_BIND_METHOD(Terrain, setText,          DEF_METHOD("setText"));
        CLASS_BIND_METHOD(Terrain, getFont,          DEF_METHOD("getFont"));
        CLASS_BIND_METHOD(Terrain, setFont,          DEF_METHOD("setFont"));
        CLASS_BIND_METHOD(Terrain, getFontSize,        DEF_METHOD("getFontSize"));
        CLASS_BIND_METHOD(Terrain, setFontSize,        DEF_METHOD("setFontSize"));
        CLASS_BIND_METHOD(Terrain, getWidth,         DEF_METHOD("getWidth"));
        CLASS_BIND_METHOD(Terrain, setWidth,         DEF_METHOD("setWidth"));
        CLASS_BIND_METHOD(Terrain, getHeight,        DEF_METHOD("getHeight"));
        CLASS_BIND_METHOD(Terrain, setHeight,        DEF_METHOD("setHeight"));
        
        CLASS_REGISTER_PROPERTY(Terrain, "Width", Variant::Type::Int, "getWidth", "setWidth");
        CLASS_REGISTER_PROPERTY(Terrain, "Height", Variant::Type::Int, "getHeight", "setHeight");
        CLASS_REGISTER_PROPERTY(Terrain, "Text", Variant::Type::String, "getText", "setText");
        CLASS_REGISTER_PROPERTY(Terrain, "Font", Variant::Type::ResourcePath, "getFont", "setFont");
        CLASS_REGISTER_PROPERTY(Terrain, "FontSize", Variant::Type::Int, "getFontSize", "setFontSize");
    }
    
    void Terrain::setText(const String& text)
    {
        m_text = StringUtil::MBS2WCS(text);
        buildRenderable();
    }
    
    void Terrain::setFont(const ResourcePath& path)
    {
        if (m_fontRes.setPath(path.getPath()))
        {
            buildRenderable();
        }
    }
    
    void Terrain::setFontSize(i32 fontSize)
    {
        m_fontSize = fontSize;
        if (m_fontSize > 0)
        {
            buildRenderable();
        }
    }
    
    void Terrain::setWidth(i32 width)
    {
        if (m_width != width)
        {
            m_width = width;
            
            buildRenderable();
        }
    }
    
    void Terrain::setHeight(i32 height)
    {
        if (m_height != height)
        {
            m_height = height;
            
            buildRenderable();
        }
    }
    
    void Terrain::buildRenderable()
    {
        if (!m_text.empty())
        {
            clearRenderable();
            
            // material
            m_material = ECHO_CREATE_RES(Material);
            //m_material->setShaderContent("echo_text_default_shader", UiMaterial::getDefault());
            m_material->setRenderStage("Transparent");
            
            // mesh
            VertexArray vertices;
            IndiceArray indices;
            buildMeshData(vertices, indices);
            
            MeshVertexFormat define;
            define.m_isUseUV = true;
            
            m_mesh = Mesh::create(true, true);
            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);
            
            m_renderable = Renderable::create(m_mesh, m_material, this);
        }
    }
    
    void Terrain::update_self()
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
    
    void Terrain::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
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
                    oVertices.push_back(VertexFormat(Vector3(left, top, 0.f), Vector2(uvLeft, uvTop)));
                    oVertices.push_back(VertexFormat(Vector3(left, bottom, 0.f), Vector2(uvLeft, uvBottom)));
                    oVertices.push_back(VertexFormat(Vector3(right, bottom, 0.f), Vector2(uvRight, uvBottom)));
                    oVertices.push_back(VertexFormat(Vector3(right, top, 0.f), Vector2(uvRight, uvTop)));
                    
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
        for (Terrain::VertexFormat& vert : oVertices)
            m_localAABB.addPoint(vert.m_position);
    }
    
    void Terrain::updateMeshBuffer()
    {
        VertexArray    vertices;
        IndiceArray    indices;
        buildMeshData(vertices, indices);
        
        MeshVertexFormat define;
        define.m_isUseUV = true;
        
        m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
        m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);
    }
    
    void Terrain::clear()
    {
        clearRenderable();
    }
    
    void Terrain::clearRenderable()
    {
        EchoSafeRelease(m_renderable);
        EchoSafeRelease(m_material);
        EchoSafeRelease(m_mesh);
    }
}
