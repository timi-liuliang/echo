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
        static i32 syncAll();
        
    public:
        Channel(Object* owner, const String& name, const String& expression);
        ~Channel();
        
        // get name
        const String& getName() const { return m_name; }
        
        // get expression
        const String& getExpression() const { return m_expression; }
        
    protected:
        Object*     m_owner = nullptr;
        String      m_name;
        String      m_expression;
    };
    typedef std::vector<Channel*>* ChannelsPtr;
}
