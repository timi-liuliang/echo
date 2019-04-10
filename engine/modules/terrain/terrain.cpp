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
        CLASS_BIND_METHOD(Terrain, getColumns,       DEF_METHOD("getColumns"));
        CLASS_BIND_METHOD(Terrain, getRows,          DEF_METHOD("getRows"));
		CLASS_BIND_METHOD(Terrain, getHeightRange,   DEF_METHOD("getHeightRange"));
		CLASS_BIND_METHOD(Terrain, setHeightRange,   DEF_METHOD("setHeightRange"));
        
        CLASS_REGISTER_PROPERTY(Terrain, "Heightmap", Variant::Type::ResourcePath, "getHeightmap", "setHeightmap");
		CLASS_REGISTER_PROPERTY(Terrain, "HeightRange", Variant::Type::Real, "getHeightRange", "setHeightRange");
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

                buildRenderable();
            }
        }
    }

	void Terrain::setHeightRange(float range)
	{ 
		m_heightRange = range;

		buildRenderable();
	}
    
    void Terrain::buildRenderable()
    {
        if (m_heightmapImage && m_width > 0 && m_height > 0)
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
            define.m_isUseNormal = true;
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
            for(i32 column=0; column <m_width; column++)
            {
                for(i32 row=0; row <m_height; row++)
                {
                    VertexFormat vert;
                    vert.m_position = Vector3(column, getHeight(column, row), row);
                    vert.m_uv = Vector2(column, row);
                    vert.m_normal = getNormal(column, row);
                    oVertices.push_back(vert);
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
        define.m_isUseNormal = true;
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
    
    float Terrain::getHeight(i32 x, i32 z)
    {
        if(m_heightmapImage)
        {
            i32 column = Math::Clamp(x, 0, m_width);
            i32 row = Math::Clamp(z, 0, m_height);
            Color color = m_heightmapImage->getColor(column, row, 0);
            float height = (color.r * 2.f - 1.f) * m_heightRange;
            
            return height;
        }
        
        return 0.f;
    }
    
    Vector3 Terrain::getNormal( i32 x, i32 z)
    {
        if(m_heightmapImage)
        {
            float h0 = getHeight(   x,   z);
            float h1 = getHeight( x+1,   z) - h0;
            float h2 = getHeight(   x, z-1) - h0;
            float h3 = getHeight( x-1,   z) - h0;
            float h4 = getHeight(   x, z+1) - h0;
            
            Vector3 normal( h3 - h1, 2, h2 - h4);
            normal.normalize();
            
            return normal;
        }

        return Vector3::UNIT_Y;
    }
}
