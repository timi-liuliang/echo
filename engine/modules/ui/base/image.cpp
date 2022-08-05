#include "image.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/ui_module.h"

namespace Echo
{
    UiImage::UiImage()
        : UiRender()
    {
    }
    
    UiImage::~UiImage()
    {
        clear();
    }
    
    void UiImage::bindMethods()
    {
        CLASS_BIND_METHOD(UiImage, getWidth);
        CLASS_BIND_METHOD(UiImage, setWidth);
        CLASS_BIND_METHOD(UiImage, getHeight);
        CLASS_BIND_METHOD(UiImage, setHeight);
        CLASS_BIND_METHOD(UiImage, getAnchor);
        CLASS_BIND_METHOD(UiImage, setAnchor);
        CLASS_BIND_METHOD(UiImage, getTextureRes);
        CLASS_BIND_METHOD(UiImage, setTextureRes);
        CLASS_BIND_METHOD(UiImage, getColor);
        CLASS_BIND_METHOD(UiImage, setColor);
        CLASS_BIND_METHOD(UiImage, getMaterial);
        CLASS_BIND_METHOD(UiImage, setMaterial);
        
        CLASS_REGISTER_PROPERTY(UiImage, "Width", Variant::Type::Int, getWidth, setWidth);
        CLASS_REGISTER_PROPERTY(UiImage, "Height", Variant::Type::Int, getHeight, setHeight);
        CLASS_REGISTER_PROPERTY(UiImage, "Anchor", Variant::Type::Vector2, getAnchor, setAnchor);
        CLASS_REGISTER_PROPERTY(UiImage, "Texture", Variant::Type::ResourcePath, getTextureRes, setTextureRes);
        CLASS_REGISTER_PROPERTY(UiImage, "Color", Variant::Type::Color, getColor, setColor);
        CLASS_REGISTER_PROPERTY(UiImage, "Material", Variant::Type::Object, getMaterial, setMaterial);
        CLASS_REGISTER_PROPERTY_HINT(UiImage, "Material", PropertyHintType::ObjectType, "Material");
    }
    
    void UiImage::setWidth(i32 width)
    {
        if (m_width != width)
        {
            m_width = width;
            
            clearRenderable();
        }
    }
    
    void UiImage::setHeight(i32 height)
    {
        if (m_height != height)
        {
            m_height = height;
            
            clearRenderable();
        }
    }

    void UiImage::setAnchor(const Vector2& anchor)
    {
        if (m_anchor != anchor)
        {
            m_anchor = anchor;

            clearRenderable();
        }
    }

    void UiImage::setTextureRes(const ResourcePath& path)
    {
        if (m_textureRes.setPath(path.getPath()))
        {
            clearRenderable();
        }
    }

    void UiImage::setColor(const Color& color)
    {
        if (m_color != color)
        {
            m_color = color;

            clearRenderable();
        }
    }

    void UiImage::setMaterial(Object* material)
    { 
        if (m_material != material)
        {
            m_material = (Material*)material;
            clearRenderable();
        }    
    }
    
    void UiImage::buildRenderable()
    {
        clearRenderable();
                     
        // Material
        if(!m_material)
        {
            const ResourcePath& defaultShader = UiModule::instance()->getUiImageDefaultShader();

            m_material = ECHO_CREATE_RES(Material);
            m_material->setShaderPath(defaultShader);
        }

        if (!m_textureRes.getPath().empty() && m_material && m_material->isUniformExist("SrcTexture"))
            m_material->setUniformTexture("SrcTexture", m_textureRes.getPath());

        if (m_material && m_material->isUniformExist("SrcColor"))
            m_material->setUniformValue("SrcColor", &m_color);
            
        // Mesh
        Ui::VertexArray vertices;
        Ui::IndiceArray indices;
        buildMeshData(vertices, indices);
            
        MeshVertexFormat define;
        define.m_isUseUV = true;
            
        m_mesh = Mesh::create(true, true);
        m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
        m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
            
        m_renderable = RenderProxy::create(m_mesh, m_material, this, false);
    }
    
    void UiImage::updateInternal(float elapsedTime)
    {
        if (isNeedRender())
        {
            if (!m_renderable)
                buildRenderable();
        }

        if (m_renderable)
            m_renderable->setSubmitToRenderQueue(isNeedRender());
    }
    
    void UiImage::buildMeshData(Ui::VertexArray& oVertices, Ui::IndiceArray& oIndices)
    {    
        float hw = m_width * 0.5f;
        float hh = m_height * 0.5f;

        Vector3 offset = Vector3(m_anchor.x, m_anchor.y, 0.0);
        
        // vertices
        oVertices.emplace_back(Vector3(-hw, -hh, 0.f) + offset, Vector2(0.f, 1.f));
        oVertices.emplace_back(Vector3(-hw,  hh, 0.f) + offset, Vector2(0.f, 0.f));
        oVertices.emplace_back(Vector3(hw,   hh, 0.f) + offset, Vector2(1.f, 0.f));
        oVertices.emplace_back(Vector3(hw,  -hh, 0.f) + offset, Vector2(1.f, 1.f));
        
        // calc aabb
        m_localAABB.reset();
        for (Ui::VertexFormat& vert : oVertices)
            m_localAABB.addPoint(vert.m_position);
        
        // indices
        oIndices.emplace_back(0);
        oIndices.emplace_back(1);
        oIndices.emplace_back(2);
        oIndices.emplace_back(0);
        oIndices.emplace_back(2);
        oIndices.emplace_back(3);
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
        m_renderable.reset();
        m_mesh.reset();
		m_material.reset();
    }
}
