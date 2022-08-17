#include "light_module.h"
#include "light/light.h"
#include "light/point_light.h"
#include "light/spot_light.h"
#include "light/direction_light.h"
#include "light/cube_light_custom.h"
#include "light/cube_light_capture.h"
#include "light/queue/deferred_lighting.h"
#include "light/queue/direct_lighting.h"
#include "light/queue/point_lighting.h"
#include "light/queue/spot_lighting.h"
#include "shadow/shadow_depth.h"
#include "editor/point_light_editor.h"
#include "editor/spot_light_editor.h"
#include "editor/direction_light_editor.h"
#include "editor/cube_light_capture_editor.h"
#include "editor/cube_light_custom_editor.h"
#include "editor/queue/shader_node_direction_light_attribute.h"

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
        CLASS_BIND_METHOD(LightModule, isIBLEnable);
        CLASS_BIND_METHOD(LightModule, setIBLEnable);
        CLASS_BIND_METHOD(LightModule, getIBLBrdfPath);
        CLASS_BIND_METHOD(LightModule, setIBLBrdfPath);

        CLASS_REGISTER_PROPERTY(LightModule, "ImageBasedLighting", Variant::Type::Bool, isIBLEnable, setIBLEnable);
        CLASS_REGISTER_PROPERTY(LightModule, "Brdf", Variant::Type::ResourcePath, getIBLBrdfPath, setIBLBrdfPath);
	}

	void LightModule::registerTypes()
	{
		Class::registerType<Light>();
        Class::registerType<PointLight>();
        Class::registerType<SpotLight>();
        Class::registerType<DirectionLight>();
		Class::registerType<CubeLightCustom>();
		Class::registerType<CubeLightCapture>();
        Class::registerType<DeferredLighting>();
        Class::registerType<DirectLighting>();
        Class::registerType<PointLighting>();
        Class::registerType<SpotLighting>();
        Class::registerType<ShadowDepth>();

    #ifdef ECHO_EDITOR_MODE
        Class::registerType<ShaderNodeDirectionLightAttribute>();
    #endif

        CLASS_REGISTER_EDITOR(PointLight, PointLightEditor)
        CLASS_REGISTER_EDITOR(SpotLight, SpotLightEditor)
        CLASS_REGISTER_EDITOR(DirectionLight, DirectionLightEditor)
        CLASS_REGISTER_EDITOR(CubeLightCustom, CubeLightCustomEditor)
        CLASS_REGISTER_EDITOR(CubeLightCapture, CubeLightCaptureEditor)
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
