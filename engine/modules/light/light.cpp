#include "light.h"

namespace Echo
{
	Light::Light()
		: m_isIBLEnable(true)
		, m_iblBrdfPath("", ".png")
		, m_iblDiffuseTexture(nullptr)
		, m_iblSpecularTexture(nullptr)
		, m_iblBrdfTexture(nullptr)
	{

	}

	Light::~Light()
	{

	}

	void Light::bindMethods()
	{
		CLASS_BIND_METHOD(Light, isIBLEnable, DEF_METHOD("isIBLEnable"));
		CLASS_BIND_METHOD(Light, setIBLEnable, DEF_METHOD("setIBLEnable"));
		CLASS_BIND_METHOD(Light, getIBLBrdfPath, DEF_METHOD("getIBLBrdfPath"));
		CLASS_BIND_METHOD(Light, setIBLBrdfPath, DEF_METHOD("setIBLBrdfPath"));

		CLASS_REGISTER_PROPERTY(Light, "ImageBasedLighting", Variant::Type::Bool, "isIBLEnable", "setIBLEnable");
		CLASS_REGISTER_PROPERTY(Light, "Brdf", Variant::Type::ResourcePath, "getIBLBrdfPath", "setIBLBrdfPath");
	}

	Light* Light::instance()
	{
		static Light* inst = EchoNew(Light);
		return inst;
	}

	void Light::setIBLBrdfPath(const ResourcePath& brdf)
	{
		if (m_iblBrdfPath.setPath(brdf.getPath()))
		{
			m_iblBrdfTexture = ECHO_DOWN_CAST<Texture*>(Res::get(m_iblBrdfPath));
		}
	}

	Texture* Light::getIBLDiffuseTexture() 
	{ 
		return m_iblDiffuseTexture; 
	}

	Texture* Light::getIBLSpecularTexture() 
	{ 
		return m_iblSpecularTexture; 
	}

	Texture* Light::getIBLBrdfTexture() 
	{ 
		return m_iblBrdfTexture; 
	}
}