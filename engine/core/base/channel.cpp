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
        String luaStr = StringUtil::Format(
            "channels._%d = function()\n"\
                "local self_node = objs._%d"\
                "\n"\
                "\n" \
            "end\n", m_id, m_owner->getId());
        
        LuaBinder::instance()->execString(luaStr);
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
