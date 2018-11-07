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
		Signal*				m_signal;
		Object*				m_target;
		ClassMethodBind*	m_method;

		Connect(Signal* signal, Object* target, ClassMethodBind* method)
			: m_signal(signal)
			, m_target(target)
			, m_method(method)
		{}

		// emit
		void emit(const Variant** args, int argCount);
	};

	// Signal
	class Signal
	{
	public:
		// connect
		bool connect(Object* obj, const Echo::String& methodName);
		bool connect(Object* obj, ClassMethodBind* method);

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
				connect.emit(nullptr, 0);
			}
		}
	};
}