#pragma once

#include "engine/core/main/module.h"
#include "engine/core/render/base/texture/texture.h"

namespace Echo
{
	class LightModule : public Module
	{
		ECHO_SINGLETON_CLASS(LightModule, Module)

        friend class CubeLightCustom;
        friend class CubeLightCapture;
        
	public:
		LightModule();
        ~LightModule();

		// instance
		static LightModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
        
    public:
        // image based lighting
        bool isIBLEnable() const { return m_isIBLEnable; }
        void setIBLEnable(bool isEnable) { m_isIBLEnable = isEnable; }
        
        // cube map
        const ResourcePath& getIBLBrdfPath() const { return m_iblBrdfPath; }
        void setIBLBrdfPath(const ResourcePath& brdf);
        
        // query get ibl textuer
        Texture* getIBLDiffuseTexture();
        Texture* getIBLSpecularTexture();
        Texture* getIBLBrdfTexture();
        
    protected:
        bool            m_isIBLEnable = true;
        ResourcePath    m_iblBrdfPath = ResourcePath("", ".png");
        Texture*        m_iblDiffuseTexture = nullptr;
        Texture*        m_iblSpecularTexture = nullptr;
        Texture*        m_iblBrdfTexture = nullptr;
	};
}
