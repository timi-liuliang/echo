#pragma once

#include "engine/core/main/module.h"
#include "engine/core/render/base/texture.h"

namespace Echo
{
	class HitProxyModule : public Module
	{
		ECHO_SINGLETON_CLASS(HitProxyModule, Module)
        
	public:
        HitProxyModule();
        ~HitProxyModule();

		// instance
		static HitProxyModule* instance();

		// register all types of the module
		virtual void registerTypes() override;  
        
    protected:
	};
}
