#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/renderable.h"

namespace Echo
{
    class ParticleSystem : public Render
    {
        ECHO_CLASS(ParticleSystem, Render)

    public:
        struct VertexFormat
        {
            Vector3        m_position;
            Vector2        m_uv;

            VertexFormat(const Vector3& pos, const Vector2& uv)
                : m_position(pos), m_uv(uv)
            {}
        };
        typedef vector<VertexFormat>::type    VertexArray;
        typedef vector<Word>::type    IndiceArray;

    public:
        ParticleSystem();
        virtual ~ParticleSystem();

        // material
        Material* getMaterial() const { return m_material; }
        void setMaterial(Object* material);

    protected:
        // build drawable
        void buildRenderable();

        // update
        virtual void update_self() override;

        // update vertex buffer
        void updateMeshBuffer();

    private:
        bool                       m_isRenderableDirty = true;
        MeshPtr                    m_mesh;                        // Geometry Data for render
        MaterialPtr                m_material;                    // Material Instance
        Renderable*                m_renderable = nullptr;
    };
}
