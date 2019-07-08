#include "box2d_module.h"
#include "box2d_world.h"
#include "box2d_body.h"
#include "box2d_shape_box.h"
#include "box2d_shape_circle.h"
#include "box2d_joint.h"
#include "box2d_joint_distance.h"
#include "box2d_joint_friction.h"
#include "box2d_joint_gear.h"
#include "box2d_joint_motor.h"
#include "box2d_joint_mouse.h"
#include "box2d_joint_prismatic.h"
#include "box2d_joint_pulley.h"
#include "box2d_joint_revolute.h"
#include "box2d_joint_rope.h"
#include "box2d_joint_weld.h"
#include "box2d_joint_wheel.h"

namespace Echo
{
	Box2DModule::Box2DModule()
	{
	}

	Box2DModule::~Box2DModule()
	{
		EchoSafeDeleteInstance(Box2DWorld);
	}

	void Box2DModule::registerTypes()
	{
		Class::registerType<Box2DWorld>();
		Class::registerType<Box2DBody>();
		Class::registerType<Box2DShape>();
		Class::registerType<Box2DShapeBox>();
		Class::registerType<Box2DShapeCircle>();
        
        // eleven joints
        Class::registerType<Box2DJoint>();
        Class::registerType<Box2DJointDistance>();
        Class::registerType<Box2DJointFriction>();
        Class::registerType<Box2DJointGear>();
        Class::registerType<Box2DJointMotor>();
        Class::registerType<Box2DJointMouse>();
        Class::registerType<Box2DJointPrismatic>();
        Class::registerType<Box2DJointPulley>();
        Class::registerType<Box2DJointRevolute>();
        Class::registerType<Box2DJointRope>();
        Class::registerType<Box2DJointWeld>();
        Class::registerType<Box2DJointWheel>();
	}

	void Box2DModule::update(float elapsedTime)
	{
		Box2DWorld::instance()->setp(elapsedTime);
	}
}
