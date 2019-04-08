#include "terrain.h"
#include "terrain_material.h"
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
    {
        set2d(false);
    }
    
    Terrain::~Terrain()
    {
        clear();
    }
    
    void Terrain::bindMethods()
    {
        CLASS_BIND_METHOD(Terrain, getHeightmap,     DEF_METHOD("getHeightmap"));
        CLASS_BIND_METHOD(Terrain, setHeightmap,     DEF_METHOD("setHeightmap"));
        CLASS_BIND_METHOD(Terrain, getWidth,         DEF_METHOD("getWidth"));
        CLASS_BIND_METHOD(Terrain, getHeight,        DEF_METHOD("getHeight"));
        
        CLASS_REGISTER_PROPERTY(Terrain, "Heightmap", Variant::Type::ResourcePath, "getHeightmap", "setHeightmap");
    }
    
    void Terrain::setHeightmap(const ResourcePath& path)
    {
        if (m_heightmap.setPath(path.getPath()))
        {
            m_heightmapImage = Image::loadFromFile( path.getPath());
            if (m_heightmapImage)
            {
                m_width = m_heightmapImage->getWidth();
                m_height = m_heightmapImage->getHeight();
                if(m_width>0 && m_height>0)
                    buildRenderable();
            }
        }
    }
    
    void Terrain::buildRenderable()
    {
        if (m_heightmapImage)
        {
            clearRenderable();
            
            // material
            m_material = ECHO_CREATE_RES(Material);
            m_material->setShaderContent("echo_terrain_default_shader", TerrainMaterial::getDefault());
            m_material->setRenderStage("Opaque");
            
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
        if(m_width>0 && m_height>0)
        {
            // vertex buffer
            for(i32 w=0; w<m_width; w++)
            {
                for(i32 h=0; h<m_height; h++)
                {
                    float height = m_heightmapImage->getColor(w, h, 0).r;
                    oVertices.push_back(VertexFormat(Vector3(w, h, height), Vector2(w, h)));
                }
            }
            
            // index buffer
            for(i32 w=0; w<m_width; w++)
            {
                for(i32 h=0; h<m_height; h++)
                {
                    i32 indexLeftTop = h * m_width + w;
                    i32 indexRightTop = indexLeftTop + 1;
                    i32 indexLeftBottom = indexLeftTop + m_width;
                    i32 indexRightBottom = indexRightTop + m_width;
                    
                    oIndices.push_back(indexLeftTop);
                    oIndices.push_back(indexRightTop);
                    oIndices.push_back(indexRightBottom);
                    oIndices.push_back(indexLeftTop);
                    oIndices.push_back(indexRightBottom);
                    oIndices.push_back(indexLeftBottom);
                }
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
