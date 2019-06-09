#include "signal.h"
#include "class.h"
#include "variant.h"

namespace Echo
{
	void ConnectClassMethod::emitSignal(const Variant** args, int argCount)
	{
		Variant::CallError error;
		m_method->call(m_target,  args, argCount, error);
	}
    
    void ConnectLuaMethod::emitSignal(const Variant** args, int argCount)
    {
        if(m_target)
            m_target->callLuaFunction( m_functionName, args, argCount);
    }

	bool Signal::connect(Object* obj, const Echo::String& methodName)
	{
		ClassMethodBind* method = Class::getMethodBind(obj->getClassName(), methodName);
		return method ? connectClassMethod(obj, method) : connectLuaMethod(obj, methodName);
	}

	bool Signal::connectClassMethod(Object* obj, ClassMethodBind* method)
	{
		m_connects.push_back(ConnectClassMethod(this, obj, method));

		return true;
	}
    
    bool Signal::connectLuaMethod(Object* obj, const Echo::String& luaMethodName)
    {
        m_connects.push_back(ConnectLuaMethod(this, obj, luaMethodName));
        
        return true;
    }
}
