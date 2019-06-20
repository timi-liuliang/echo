#include "channel.h"
#include "engine/core/script/lua/lua_binder.h"
#include "object.h"

namespace Echo
{
    Channel::Channel(Object* owner, const String& name, const String& expression)
        : m_owner(owner)
        , m_name(name)
        , m_expression(expression)
    {
        // unique id
        static i32 id = 0;
        m_id = id++;
        
        // register to lua
        registerToLua();
    }
    
    Channel::~Channel()
    {
        unregisterFromLua();
    }
    
    void Channel::registerToLua()
    {
        String getExpression = StringUtil::Replace(m_expression, "ch(", StringUtil::Format("objs._%d:ch(", m_owner->getId()));
        PropertyInfoStatic* propertyInfo = ECHO_DOWN_CAST<PropertyInfoStatic*>(Class::getProperty(m_owner, m_name));
        if(propertyInfo)
        {
            String luaStr = StringUtil::Format
            (
                "channels._%d = function()\n"\
                "    local result = %s\n"\
                "    objs._%d:%s(result)\n"\
                "end\n", m_id, getExpression.c_str(), m_owner->getId(), propertyInfo->m_setter.c_str()
             );
            
            LuaBinder::instance()->execString(luaStr);
        }
    }
    
    void Channel::unregisterFromLua()
    {
        String luaStr = StringUtil::Format("channels._%d = nil", m_id);
        LuaBinder::instance()->execString(luaStr);
    }
    
    void Channel::syncAll()
    {
        LuaBinder::instance()->execString("update_all_channels()", true);
    }
}
