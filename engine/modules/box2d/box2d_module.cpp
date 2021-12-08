#include "box2d_module.h"
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
#include "engine/core/main/Engine.h"

namespace Echo
{
	DECLARE_MODULE(Box2DModule)

	Box2DModule::Box2DModule()
        : m_drawDebugOption("Editor", {"None","Editor","Game","All"})
	{
        m_isGame = Engine::instance()->getConfig().m_isGame;
        m_b2World = EchoNew(b2World(b2Vec2(m_gravity.x, m_gravity.y)));

        // debug draw
        m_debugDraw = EchoNew(Box2DDebugDraw);
        m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit /* |b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit*/);
        m_b2World->SetDebugDraw(m_debugDraw);
        
        // contact listener
        m_contactListener = EchoNew(Box2DContactListener);
        m_b2World->SetContactListener(m_contactListener);
	}

	Box2DModule::~Box2DModule()
	{
        EchoSafeDelete(m_b2World, b2World);
        EchoSafeDelete(m_debugDraw, Box2DDebugDraw);
        EchoSafeDelete(m_contactListener, Box2DContactListener);
	}

	Box2DModule* Box2DModule::instance()
	{
		static Box2DModule* inst = EchoNew(Box2DModule);
		return inst;
	}

	void Box2DModule::bindMethods()
	{
        CLASS_BIND_METHOD(Box2DModule, getFramesPerSecond);
        CLASS_BIND_METHOD(Box2DModule, setFramesPerSecond);
        CLASS_BIND_METHOD(Box2DModule, getPixelsPerMeter);
        CLASS_BIND_METHOD(Box2DModule, setPixelsPerPeter);
        CLASS_BIND_METHOD(Box2DModule, getGravity);
        CLASS_BIND_METHOD(Box2DModule, setGravity);
        CLASS_BIND_METHOD(Box2DModule, getDebugDrawOption);
        CLASS_BIND_METHOD(Box2DModule, setDebugDrawOption);

        CLASS_REGISTER_PROPERTY(Box2DModule, "FramesPerSecond", Variant::Type::Int, getFramesPerSecond, setFramesPerSecond);
        CLASS_REGISTER_PROPERTY(Box2DModule, "DebugDraw", Variant::Type::StringOption, getDebugDrawOption, setDebugDrawOption);
        CLASS_REGISTER_PROPERTY(Box2DModule, "PixelsPerMeter", Variant::Type::Real, getPixelsPerMeter, setPixelsPerMeter);
        CLASS_REGISTER_PROPERTY(Box2DModule, "Gravity", Variant::Type::Vector2, getGravity, setGravity);
	}

	void Box2DModule::registerTypes()
	{
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

    void Box2DModule::setGravity(const Vector2& gravity)
    {
        m_gravity = gravity;
        if (m_b2World)
        {
            m_b2World->SetGravity(b2Vec2( m_gravity.x, m_gravity.y));
        }
    }

	void Box2DModule::update(float elapsedTime)
	{
        if (m_isGame)
        {
            float timeStep = 1.f / m_framesPerSecond;

            static float elapsedFixed = 0.f;
            elapsedFixed += elapsedTime;
            while (elapsedFixed >= timeStep)
            {
                //move the world ahead , step ahead man!!
                m_b2World->Step(timeStep, 8, 3);
                m_b2World->ClearForces();

                elapsedFixed -= timeStep;
            }
        }

        // draw debug data
        bool isGame = Engine::instance()->getConfig().m_isGame;
        if(m_drawDebugOption.getIdx() == 3 || (m_drawDebugOption.getIdx() == 1 && !isGame) || (m_drawDebugOption.getIdx() == 2 && isGame) )
        {
            m_b2World->DrawDebugData();
            m_debugDraw->Update(elapsedTime);
        }
        
        // emit signals
        m_contactListener->EmitSignals();
	}
}
