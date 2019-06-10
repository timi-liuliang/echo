#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class Variant;
	class Signal;
	class Object;
	class ClassMethodBind;

	// Connect
	struct Connect
	{
        String        m_targetPath;
        String        m_functionName;
        
        virtual void emitSignal(const Variant** args, int argCount) {}
        
        // save
        void save(void* pugiNode);
	};
    
    struct ConnectClassMethod : public Connect
    {
        Signal*             m_signal;
        Object*             m_target;
        ClassMethodBind*    m_method;
        
        ConnectClassMethod(Signal* signal, Object* target, ClassMethodBind* method)
        : m_signal(signal)
        , m_target(target)
        , m_method(method)
        {}
        
        // emit
        virtual void emitSignal(const Variant** args, int argCount) override;
    };
    
    struct ConnectLuaMethod : public Connect
    {
        Signal*           m_signal;
        Object*           m_target;

        ConnectLuaMethod(Signal* signal, Object* target, const String& functionName)
        : m_signal(signal)
        , m_target(target)
        {
            m_functionName = functionName;
        }
        
        // emit
        virtual void emitSignal(const Variant** args, int argCount) override;
    };

	// Signal
    // Reference: https://github.com/pbhogan/Signals
    // A lightweight signals and slots implementation
	class Signal
	{
	public:
		// connect
        bool connect(const String& obj, const String& method);
		bool connect(Object* obj, const Echo::String& methodName);
		bool connectClassMethod(Object* obj, ClassMethodBind* method);
        bool connectLuaMethod(Object* obj, const Echo::String& luaMethodName);
        
        // is have connect
        bool isHaveConnects() { return !m_connects.empty(); }
        
        // load/save
        void load(void* pugiNode);
        void save(void* pugiNode);

	protected:
		vector<Connect>::type	m_connects;
	};

	class Signal0 : public Signal
	{
	public:
		// operate ()
		void operator() ()
		{
			for (Connect& connect : m_connects)
			{
				connect.emitSignal(nullptr, 0);
			}
		}
	};
}

#define DECLARE_SIGNAL(type, signal) \
    Signal* getSignal##signal() { return &signal; } \
    type    signal;


