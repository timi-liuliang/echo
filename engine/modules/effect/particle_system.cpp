#include "particle_system.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
    ParticleSystem::ParticleSystem()
        : Render()
    {
    }

    ParticleSystem::~ParticleSystem()
    {
        EchoSafeRelease(m_renderable);
        m_mesh.reset();
    }

    void ParticleSystem::bindMethods()
    {
        CLASS_BIND_METHOD(ParticleSystem, getMaterial,        DEF_METHOD("getMaterial"));
        CLASS_BIND_METHOD(ParticleSystem, setMaterial,        DEF_METHOD("setMaterial"));

        CLASS_REGISTER_PROPERTY(Sprite, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
        CLASS_REGISTER_PROPERTY_HINT(Sprite, "Material", PropertyHintType::ResourceType, "Material");
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
            EchoSafeRelease(m_renderable);

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
            m_renderable = Renderable::create(m_mesh, m_material, this);

            m_isRenderableDirty = false;
        }
    }

    void ParticleSystem::update_self()
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
            vertices.push_back(VertexFormat(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f)));
            vertices.push_back(VertexFormat(Vector3(-hw,  hh, 0.f), Vector2(0.f, 0.f)));
            vertices.push_back(VertexFormat(Vector3(hw,   hh, 0.f), Vector2(1.f, 0.f)));
            vertices.push_back(VertexFormat(Vector3(hw,  -hh, 0.f), Vector2(1.f, 1.f)));

            // format
            MeshVertexFormat define;
            define.m_isUseUV = true;

            m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

            m_localAABB = m_mesh->getLocalBox();
        }
    }
}
