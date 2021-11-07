#include "shader_template.h"

namespace Echo
{  
    ShaderTemplate::ShaderTemplate()
	{
	}

    ShaderTemplate::ShaderTemplate(const ResourcePath& path)
		: Res(path)
	{

	}

    ShaderTemplate::~ShaderTemplate()
	{
	}

	void ShaderTemplate::bindMethods()
	{
        CLASS_BIND_METHOD(ShaderTemplate, getVsCode, DEF_METHOD("getVsCode"));
        CLASS_BIND_METHOD(ShaderTemplate, setVsCode, DEF_METHOD("setVsCode"));
        CLASS_BIND_METHOD(ShaderTemplate, getPsCode, DEF_METHOD("getPsCode"));
        CLASS_BIND_METHOD(ShaderTemplate, setPsCode, DEF_METHOD("setPsCode"));

        CLASS_REGISTER_PROPERTY(ShaderTemplate, "VertexShader", Variant::Type::String, "getVsCode", "setVsCode");
        CLASS_REGISTER_PROPERTY(ShaderTemplate, "FragmentShader", Variant::Type::String, "getPsCode", "setPsCode");

        CLASS_REGISTER_PROPERTY_HINT(ShaderTemplate, "VertexShader", PropertyHintType::Language, "glsl");
        CLASS_REGISTER_PROPERTY_HINT(ShaderTemplate, "FragmentShader", PropertyHintType::Language, "glsl");
	}
    
	const String& ShaderTemplate::getVsCode() const
    { 
        return m_vsCode;
    }

    void ShaderTemplate::setVsCode(const String& vsCode)
    {
        m_vsCode=vsCode;
    }

    void ShaderTemplate::setPsCode(const String& psCode)
    {
        m_psCode=psCode;
    }

	const String& ShaderTemplate::getPsCode() const
    { 
        return m_psCode; 
    }
}
