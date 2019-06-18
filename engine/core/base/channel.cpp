#include "channel.h"

namespace Echo
{
    static std::vector<Channel*> g_channels;
    
    Channel::Channel(Object* owner, const String& name, const String& expression)
        : m_owner(owner)
        , m_name(name)
        , m_expression(expression)
    {
        g_channels.push_back(this);
    }
    
    Channel::~Channel()
    {
        std::remove(g_channels.begin(), g_channels.end(), this);
    }
    
    i32 Channel::syncAll()
    {
        for(Channel* channel : g_channels)
        {
            int a = 10;
        }
    }
}
