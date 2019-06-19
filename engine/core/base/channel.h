#pragma once

#include "echo_def.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
    class Object;
    class Channel
    {
    public:
        // sync all
        static void syncAll();
        
    public:
        Channel(Object* owner, const String& name, const String& expression);
        ~Channel();
        
        // get name
        const String& getName() const { return m_name; }
        
        // get expression
        const String& getExpression() const { return m_expression; }
        
    private:
        // register to lua
        void registerToLua();
        void unregisterFromLua();
        
    protected:
        i32         m_id = 0;
        Object*     m_owner = nullptr;
        String      m_name;
        String      m_expression;
    };
    typedef std::vector<Channel*>* ChannelsPtr;
}
