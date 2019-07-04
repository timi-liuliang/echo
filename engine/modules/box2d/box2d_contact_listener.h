#pragma once

#include <Box2D/Box2D.h>

namespace Echo
{
	class Box2DContactListener : public b2ContactListener
	{
	public:
		Box2DContactListener();

        /// Called when two fixtures begin to touch.
        virtual void BeginContact(b2Contact* contact) override;
        
        /// Called when two fixtures cease to touch.
        virtual void EndContact(b2Contact* contact) override;
	};
}
