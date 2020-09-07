#include "physx_module.h"
#include "physx_world.h"
#include "shape/physx_shape.h"
#include "shape/physx_shape_sphere.h"
#include "shape/physx_shape_capsule.h"
#include "shape/physx_shape_plane.h"
#include "shape/physx_shape_heightfield.h"
#include "physx_body.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	DECLARE_MODULE(PhysxModule)

	PhysxModule::PhysxModule()
	{

	}
    
    PhysxModule::~PhysxModule()
    {
        EchoSafeDeleteInstance(PhysxWorld);
    }

	PhysxModule* PhysxModule::instance()
	{
		static PhysxModule* inst = EchoNew(PhysxModule);
		return inst;
	}

	void PhysxModule::bindMethods()
	{
        CLASS_BIND_METHOD(PhysxModule, getDebugDrawOption, DEF_METHOD("getDebugDrawOption"));
        CLASS_BIND_METHOD(PhysxModule, setDebugDrawOption, DEF_METHOD("setDebugDrawOption"));

        CLASS_REGISTER_PROPERTY(PhysxModule, "DebugDraw", Variant::Type::StringOption, "getDebugDrawOption", "setDebugDrawOption");
	}

	void PhysxModule::registerTypes()
	{
		Class::registerType<PhysxWorld>();
		Class::registerType<PhysxBody>();
		Class::registerType<PhysxShape>();
		Class::registerType<PhysxShapeSphere>();
		Class::registerType<PhysxShapeCapsule>();
		Class::registerType<PhysxShapePlane>();
		Class::registerType<PhysxShapeHeightfield>();
	}

	void PhysxModule::update(float elapsedTime)
	{
		PhysxWorld::instance()->step(elapsedTime);
	}

    void PhysxModule::setDebugDrawOption(const StringOption& option)
    {
        m_drawDebugOption.setValue(option.getValue());
    }
}
