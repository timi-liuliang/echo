#pragma once

#include <Box2D/Box2D.h>
#include <engine/core/base/object.h>

namespace Echo
{
	class Box2DContactListener : public b2ContactListener
	{
    public:
        // Signal
        struct SignalCache
        {
            ui32    m_bodyAId;
            ui32    m_bodyBId;
            bool    m_isBegin;
            
            SignalCache(i32 idA, i32 idB, bool isBegin)
                : m_bodyAId(idA), m_bodyBId(idB), m_isBegin(isBegin)
            {}
        };
        
	public:
		Box2DContactListener();

        /// Called when two fixtures begin to touch.
        virtual void BeginContact(b2Contact* contact) override;
        
        /// Called when two fixtures cease to touch.
        virtual void EndContact(b2Contact* contact) override;
        
        /// Emit
        void EmitSignals();
        
    private:
        vector<SignalCache>::type m_signals;
	};
}
