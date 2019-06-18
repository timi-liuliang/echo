#include "channel.h"

namespace Echo
{
    Channel::Channel(Object* owner, const String& name, const String& expression)
        : m_owner(owner)
        , m_name(name)
        , m_expression(expression)
    {
        
    }
    
    i32 Channel::syncAll()
    {
        return 0;
    }
}
