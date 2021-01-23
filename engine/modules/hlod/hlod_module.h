#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class HLodModule : public Module
	{
		ECHO_SINGLETON_CLASS(HLodModule, Module)
        
	public:
        HLodModule();
        virtual ~HLodModule();

		// instance
		static HLodModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
        
    protected:
	};
}
