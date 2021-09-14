#include "particle_system.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
    ParticleSystem::ParticleSystem()
        : Render()
    {
    }

    ParticleSystem::~ParticleSystem()
    {
        m_renderable.reset();
        m_mesh.reset();
    }

    void ParticleSystem::bindMethods()
    {
        CLASS_BIND_METHOD(ParticleSystem, getMaterial,        DEF_METHOD("getMaterial"));
        CLASS_BIND_METHOD(ParticleSystem, setMaterial,        DEF_METHOD("setMaterial"));

        CLASS_REGISTER_PROPERTY(ParticleSystem, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
        CLASS_REGISTER_PROPERTY_HINT(ParticleSystem, "Material", PropertyHintType::ObjectType, "Material");
    }

    void ParticleSystem::setMaterial(Object* material)
    {
        m_material = (Material*)material;

        m_isRenderableDirty = true;
    }

    void ParticleSystem::buildRenderable()
    {
        if (m_isRenderableDirty)
        {
            if (!m_material)
            {
                StringArray macros = { "ALPHA_ADJUST" };
                ShaderProgramPtr shader = ShaderProgram::getDefault2D(macros);

                m_material = ECHO_CREATE_RES(Material);
                m_material->setShaderPath(shader->getPath());
            }

            // mesh
            updateMeshBuffer();

            // create render able
            m_renderable = RenderProxy::create(m_mesh, m_material, this);

            m_isRenderableDirty = false;
        }
    }

    void ParticleSystem::updateInternal(float elapsedTime)
    {
        if (isNeedRender())
        {
            buildRenderable();
            if (m_renderable)
            {
                m_renderable->submitToRenderQueue();
            }
        }
    }

    void ParticleSystem::updateMeshBuffer()
    {
        if(!m_mesh)
        {
            m_mesh = Mesh::create(true, true);
        }
        
        if(m_mesh)
        {
            // indices
            IndiceArray indices = { 0, 1, 2, 0, 2, 3 };

            float hw = 64 * 0.5f;
            float hh = 64 * 0.5f;

            // vertices
            VertexArray vertices;
            vertices.emplace_back(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f));
            vertices.emplace_back(Vector3(-hw,  hh, 0.f), Vector2(0.f, 0.f));
            vertices.emplace_back(Vector3(hw,   hh, 0.f), Vector2(1.f, 0.f));
            vertices.emplace_back(Vector3(hw,  -hh, 0.f), Vector2(1.f, 1.f));

            // format
            MeshVertexFormat define;
            define.m_isUseUV = true;

            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

            m_localAABB = m_mesh->getLocalBox();
        }
    }
}
