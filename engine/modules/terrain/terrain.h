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
            Vector3        m_normal;
            Vector2        m_uv;
        };
        typedef vector<VertexFormat>::type  VertexArray;
        typedef vector<ui32>::type          IndiceArray;
        
    public:
        Terrain();
        virtual ~Terrain();
        
        // font res path
        void setHeightmap(const ResourcePath& path);
        const ResourcePath& getHeightmap() { return m_heightmap; }
        
        // width height
        i32 getColumns() const { return m_columns; }
        i32 getRows() const { return m_rows; }

		// height range
		float getHeightRange() const { return m_heightRange; }
		void setHeightRange(float range);

		// grid spacing
		i32 getGridSpacing() const { return m_gridSpacing; }
		void setGridSpacing(i32 gridSpacing);
        
        // get height
        float getHeight(i32 x, i32 z);
        
        // get normal
        Vector3 getNormal(i32 x, i32 z);
        
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
		float					m_heightRange = 256.f;
		i32						m_gridSpacing = 1;
        Mesh*                   m_mesh;                     // Geometry Data for render
        Material*               m_material;                 // Material Instance
        Renderable*             m_renderable;
        Matrix4                 m_matWVP;
        i32                     m_columns = 0;
        i32                     m_rows = 0;
    };
}
