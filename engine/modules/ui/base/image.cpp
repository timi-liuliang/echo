#include "image.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
    UiImage::UiImage()
    : UiRender()
	, m_textureRes("", ".png")
    , m_mesh(nullptr)
    , m_material(nullptr)
    , m_renderable(nullptr)
    , m_width(0)
    , m_height(0)
    {
    }
    
    UiImage::~UiImage()
    {
        clear();
    }
    
    void UiImage::bindMethods()
    {
        CLASS_BIND_METHOD(UiImage, getTextureRes,   DEF_METHOD("getTextureRes"));
        CLASS_BIND_METHOD(UiImage, setTextureRes,   DEF_METHOD("setTextureRes"));
        CLASS_BIND_METHOD(UiImage, getWidth,        DEF_METHOD("getWidth"));
        CLASS_BIND_METHOD(UiImage, setWidth,        DEF_METHOD("setWidth"));
        CLASS_BIND_METHOD(UiImage, getHeight,       DEF_METHOD("getHeight"));
        CLASS_BIND_METHOD(UiImage, setHeight,       DEF_METHOD("setHeight"));
        CLASS_BIND_METHOD(UiImage, getMaterial,     DEF_METHOD("getMaterial"));
        CLASS_BIND_METHOD(UiImage, setMaterial,     DEF_METHOD("setMaterial"));
        
        CLASS_REGISTER_PROPERTY(UiImage, "Width", Variant::Type::Int, "getWidth", "setWidth");
        CLASS_REGISTER_PROPERTY(UiImage, "Height", Variant::Type::Int, "getHeight", "setHeight");
        CLASS_REGISTER_PROPERTY(UiImage, "Texture", Variant::Type::ResourcePath, "getTextureRes", "setTextureRes");
        CLASS_REGISTER_PROPERTY(UiImage, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
        CLASS_REGISTER_PROPERTY_HINT(UiImage, "Material", PropertyHintType::ResourceType, "Material");
    }
    
    void UiImage::setTextureRes(const ResourcePath& path)
    {
        if (m_textureRes.setPath(path.getPath()))
        {
            buildRenderable();
        }
    }
    
    void UiImage::setWidth(i32 width)
    {
        if (m_width != width)
        {
            m_width = width;
            
            buildRenderable();
        }
    }
    
    void UiImage::setHeight(i32 height)
    {
        if (m_height != height)
        {
            m_height = height;
            
            buildRenderable();
        }
    }
    
    void UiImage::buildRenderable()
    {
        if (!m_textureRes.getPath().empty())
        {
            clearRenderable();
            
            StringArray macros = {"ALPHA_ADJUST"};
            m_shader = ShaderProgram::getDefault2D(macros);
            
            // material
            if(!m_materialDefault)
            {
                m_materialDefault = ECHO_CREATE_RES(Material);
                m_materialDefault->setShaderPath(m_shader->getPath());
            }

            m_materialDefault->getUniform("BaseColor")->setTexture(m_textureRes.getPath());
            
            // mesh
            Ui::VertexArray vertices;
            Ui::IndiceArray indices;
            buildMeshData(vertices, indices);
            
            MeshVertexFormat define;
            define.m_isUseUV = true;
            
            m_mesh = Mesh::create(true, true);
            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
            
            m_renderable = Renderable::create(m_mesh, m_materialDefault, this);
        }
    }
    
    void UiImage::update_self()
    {
        if (isNeedRender())
        {
            if (m_renderable)
            {
                m_renderable->submitToRenderQueue();
            }
        }
    }
    
    void UiImage::buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices)
    {
        //Texture* texture = m_materialDefault->getTexture(0);
        //if (texture)
        //{
        //    if(!m_width) m_width  = texture->getWidth();
        //    if(!m_height) m_height = texture->getHeight();
        //}
        
        float hw = m_width * 0.5f;
        float hh = m_height * 0.5f;
        
        // vertices
        oVertices.push_back(Ui::VertexFormat(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f)));
        oVertices.push_back(Ui::VertexFormat(Vector3(-hw,  hh, 0.f), Vector2(0.f, 0.f)));
        oVertices.push_back(Ui::VertexFormat(Vector3(hw,   hh, 0.f), Vector2(1.f, 0.f)));
        oVertices.push_back(Ui::VertexFormat(Vector3(hw,  -hh, 0.f), Vector2(1.f, 1.f)));
        
        // calc aabb
        m_localAABB.reset();
        for (Ui::VertexFormat& vert : oVertices)
            m_localAABB.addPoint(vert.m_position);
        
        // indices
        oIndices.push_back(0);
        oIndices.push_back(1);
        oIndices.push_back(2);
        oIndices.push_back(0);
        oIndices.push_back(2);
        oIndices.push_back(3);
    }
    
    void UiImage::updateMeshBuffer()
    {
        Ui::VertexArray    vertices;
        Ui::IndiceArray    indices;
        buildMeshData(vertices, indices);
        
        MeshVertexFormat define;
        define.m_isUseUV = true;
        
        m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
        m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
    }
    
    void UiImage::clear()
    {
        clearRenderable();
    }
    
    void UiImage::clearRenderable()
    {
        EchoSafeRelease(m_renderable);
        EchoSafeDelete(m_mesh, Mesh);

		m_material.reset();
    }
}
