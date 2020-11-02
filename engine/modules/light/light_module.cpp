#include "light_module.h"
#include "light.h"
#include "point_light.h"
#include "spot_light.h"
#include "direction_light.h"
#include "cube_light_custom.h"
#include "cube_light_capture.h"
#include "editor/point_light_editor.h"
#include "editor/spot_light_editor.h"
#include "editor/direction_light_editor.h"
#include "editor/cube_light_capture_editor.h"
#include "editor/cube_light_custom_editor.h"

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
        Class::registerType<PointLight>();
        Class::registerType<SpotLight>();
        Class::registerType<DirectionLight>();
		Class::registerType<CubeLightCustom>();
		Class::registerType<CubeLightCapture>();

        REGISTER_OBJECT_EDITOR(PointLight, PointLightEditor)
        REGISTER_OBJECT_EDITOR(SpotLight, SpotLightEditor)
        REGISTER_OBJECT_EDITOR(DirectionLight, DirectionLightEditor)
        REGISTER_OBJECT_EDITOR(CubeLightCustom, CubeLightCustomEditor)
        REGISTER_OBJECT_EDITOR(CubeLightCapture, CubeLightCaptureEditor)
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
