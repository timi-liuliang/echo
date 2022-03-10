#include "sprite.h"
#include "engine/core/log/log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Sprite::Sprite()
        : Render()
	{
		m_shaderDefault = ShaderProgram::getDefault2D(StringArray());
	}

	Sprite::~Sprite()
	{
		m_renderable.reset();
		m_mesh.reset();
	}

	void Sprite::bindMethods()
	{
		CLASS_BIND_METHOD(Sprite, getBillboardType);
		CLASS_BIND_METHOD(Sprite, setBillobardType);
		CLASS_BIND_METHOD(Sprite, getWidth);
		CLASS_BIND_METHOD(Sprite, setWidth);
		CLASS_BIND_METHOD(Sprite, getHeight);
		CLASS_BIND_METHOD(Sprite, setHeight);
		CLASS_BIND_METHOD(Sprite, getOffset);
		CLASS_BIND_METHOD(Sprite, setOffset);
		CLASS_BIND_METHOD(Sprite, getMaterial);
		CLASS_BIND_METHOD(Sprite, setMaterial);

		CLASS_REGISTER_PROPERTY(Sprite, "Billboard", Variant::Type::StringOption, getBillboardType, setBillobardType);
		CLASS_REGISTER_PROPERTY(Sprite, "Width", Variant::Type::Real, getWidth, setWidth);
		CLASS_REGISTER_PROPERTY(Sprite, "Height", Variant::Type::Real, getHeight, setHeight);
		CLASS_REGISTER_PROPERTY(Sprite, "Offset", Variant::Type::Vector2, getOffset, setOffset);
		CLASS_REGISTER_PROPERTY(Sprite, "Material", Variant::Type::Object, getMaterial, setMaterial);
		CLASS_REGISTER_PROPERTY_HINT(Sprite, "Material", PropertyHintType::ObjectType, "Material");
	}

	StringOption Sprite::getBillboardType()
	{
		StringOption result;
		result.fromEnum(m_billboardType);

		return result;
	}

	void Sprite::setBillobardType(const StringOption& type)
	{
		m_billboardType = type.toEnum(BillboardType::None);
	}

	void Sprite::setWidth(float width)
	{ 
		if (m_width != width)
		{
			m_width = width;
			m_isRenderableDirty = true;
		}
	}

	void Sprite::setHeight(float height)
	{
		if (m_height != height)
		{
			m_height = height;
			m_isRenderableDirty = true;
		}
	}

	void Sprite::setOffset(const Vector2& offset)
	{
		if (m_offset != offset)
		{
			m_offset = offset;
			m_isRenderableDirty = true;
		}
	}

	void Sprite::setMaterial(Object* material)
	{
		m_material = (Material*)material;

		m_isRenderableDirty = true;
	}

	void Sprite::buildRenderable()
	{
		if (m_isRenderableDirty)
		{
			if (!m_material)
			{
				m_material = ECHO_CREATE_RES(Material);
				m_material->setShaderPath(m_shaderDefault->getPath());
			}

			// mesh
			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, this, false);

			m_isRenderableDirty = false;
		}
	}

	void Sprite::updateInternal(float elapsedTime)
	{
		if (isNeedRender())
		{
			updateBillboard();

			buildRenderable();
		}

		if (m_renderable)
			m_renderable->setSubmitToRenderQueue(isNeedRender());
	}

	void Sprite::updateBillboard()
	{
		if (m_billboardType != BillboardType::None)
		{
			Camera* camera = getCamera();
			if (camera)
			{
				Vector3 faceDir = m_billboardType == BillboardType::LookAt ? getWorldPosition() - camera->getPosition() : camera->getDirection();
				faceDir.normalize();

				Vector3 hDir(faceDir.x, 0.f, faceDir.z);
				hDir.normalize();

				Quaternion quat = Quaternion::fromVec3ToVec3(hDir, faceDir) * Quaternion::fromVec3ToVec3(Vector3::UNIT_Z, hDir);
				setWorldOrientation(quat);
			}
		}
	}

	void Sprite::updateMeshBuffer()
	{
        if(!m_mesh)
        {
            if(m_width && m_height)
                m_mesh = Mesh::create(true, true);
        }
        
        if(m_mesh)
        {
			// indices
			IndiceArray indices = { 0, 1, 2, 0, 2, 3 };

            float hw = m_width * 0.5f;
            float hh = m_height * 0.5f;

			Vector3 offset(m_offset, 0.f);

            // vertices
            VertexArray vertices;
            vertices.emplace_back(Vector3(-hw, -hh, 0.f) + offset, Vector2(0.f, 1.f));
            vertices.emplace_back(Vector3(-hw,  hh, 0.f) + offset, Vector2(0.f, 0.f));
            vertices.emplace_back(Vector3(hw,   hh, 0.f) + offset, Vector2(1.f, 0.f));
            vertices.emplace_back(Vector3(hw,  -hh, 0.f) + offset, Vector2(1.f, 1.f));

            // format
            MeshVertexFormat define;
            define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
            m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			m_localAABB = m_mesh->getLocalBox();
        }
	}
}
