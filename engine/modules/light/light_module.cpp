#include "light_module.h"
#include "light.h"
#include "cube_light_custom.h"
#include "cube_light_capture.h"

namespace Echo
{
	DECLARE_MODULE(LightModule)

	LightModule::LightModule()
	{

	}
    
    LightModule::~LightModule()
    {
    }

	LightModule* LightModule::instance()
	{
		static LightModule* inst = EchoNew(LightModule);
		return inst;
	}

	void LightModule::bindMethods()
	{
        CLASS_BIND_METHOD(LightModule, isIBLEnable, DEF_METHOD("isIBLEnable"));
        CLASS_BIND_METHOD(LightModule, setIBLEnable, DEF_METHOD("setIBLEnable"));
        CLASS_BIND_METHOD(LightModule, getIBLBrdfPath, DEF_METHOD("getIBLBrdfPath"));
        CLASS_BIND_METHOD(LightModule, setIBLBrdfPath, DEF_METHOD("setIBLBrdfPath"));

        CLASS_REGISTER_PROPERTY(LightModule, "ImageBasedLighting", Variant::Type::Bool, "isIBLEnable", "setIBLEnable");
        CLASS_REGISTER_PROPERTY(LightModule, "Brdf", Variant::Type::ResourcePath, "getIBLBrdfPath", "setIBLBrdfPath");
	}

	void LightModule::registerTypes()
	{
		Class::registerType<Light>();
		Class::registerType<CubeLightCustom>();
		Class::registerType<CubeLightCapture>();
	}

    void LightModule::setIBLBrdfPath(const ResourcePath& brdf)
    {
        if (m_iblBrdfPath.setPath(brdf.getPath()))
        {
            m_iblBrdfTexture = ECHO_DOWN_CAST<Texture*>(Res::get(m_iblBrdfPath));
        }
    }

    Texture* LightModule::getIBLDiffuseTexture()
    {
        return m_iblDiffuseTexture;
    }

    Texture* LightModule::getIBLSpecularTexture()
    {
        return m_iblSpecularTexture;
    }

    Texture* LightModule::getIBLBrdfTexture()
    {
        return m_iblBrdfTexture;
    }
}
