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
        
    protected:
        Object*     m_object;
        String      m_propertyName;
        String      m_objectPathFrom;
        Object*     m_objectFrom;
        String      m_propertyNameFrom;
        String      m_expression;
    };
    typedef std::vector<Channel*>* ChannelsPtr;
}
