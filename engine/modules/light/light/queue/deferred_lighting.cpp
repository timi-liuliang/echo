#include "deferred_lighting.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	DeferredLighting::DeferredLighting()
		: IRenderQueue()
	{
	}

	DeferredLighting::~DeferredLighting()
	{

	}

	void DeferredLighting::bindMethods()
	{
		CLASS_BIND_METHOD(DeferredLighting, getMaterial);
		CLASS_BIND_METHOD(DeferredLighting, setMaterial);

		CLASS_REGISTER_PROPERTY(DeferredLighting, "Material", Variant::Type::Object, getMaterial, setMaterial);
		CLASS_REGISTER_PROPERTY_HINT(DeferredLighting, "Material", PropertyHintType::ObjectType, "Material");
	}

	void DeferredLighting::setMaterial(Object* material)
	{
		m_material = (Material*)material;
		if (m_material)
		{
		}

		m_dirty = true;
	}

	void DeferredLighting::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();


		onRenderEnd();
	}
}