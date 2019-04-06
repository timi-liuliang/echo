#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/interface/mesh/Mesh.h"
#include "engine/core/render/interface/Material.h"
#include "engine/core/render/interface/Renderable.h"
#include "engine/core/render/interface/image/Image.h"

namespace Echo
{
    class Terrain : public Render
    {
        ECHO_CLASS(Terrain, Render)
        
    public:
        // Vertex Format
        struct VertexFormat
        {
            Vector3        m_position;
            Vector2        m_uv;
            
            VertexFormat(const Vector3& pos, const Vector2& uv)
            : m_position(pos), m_uv(uv)
            {}
        };
        typedef vector<VertexFormat>::type  VertexArray;
        typedef vector<Word>::type          IndiceArray;
        
    public:
        Terrain();
        virtual ~Terrain();
        
        // font res path
        void setHeightmap(const ResourcePath& path);
        const ResourcePath& getHeightmap() { return m_heightmap; }
        
        // width height
        i32 getWidth() const { return m_width; }
        i32 getHeight() const { return m_height; }
        
    protected:
        // build drawable
        void buildRenderable();
        
        // update
        virtual void update_self() override;
        
        // update vertex buffer
        void updateMeshBuffer();
        
        // build mesh data by drawables data
        void buildMeshData(VertexArray& oVertices, IndiceArray& oIndices);
        
        // clear
        void clear();
        void clearRenderable();
        
    private:
        ResourcePath            m_heightmap = ResourcePath("", ".png");
        Image*                  m_heightmapImage = nullptr;
        Mesh*                   m_mesh;                     // Geometry Data for render
        Material*               m_material;                 // Material Instance
        Renderable*             m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_width = 0;
        i32                     m_height = 0;
    };
}
