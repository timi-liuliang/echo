#include "engine/core/log/Log.h"
#include "Material.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/base/ShaderProgram.h"
#include "engine/core/render/base/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	Material::Uniform::~Uniform()
	{
		EchoSafeDeleteArray(m_value, Byte, getValueBytes());
	}

	Material::Uniform* Material::Uniform::clone()
	{
		Uniform* result = EchoNew(Uniform);
		result->m_name = m_name;
		result->m_type = m_type;
		result->m_count = m_count;
		result->setValue( m_value);

		return result;
	}

	ui32 Material::Uniform::getValueBytes()
	{
		ui32 bytes = 0;
		switch (m_type)
		{
		case SPT_INT:		bytes = sizeof(int)*m_count;		break;
		case SPT_FLOAT:		bytes = sizeof(real32)*m_count;		break;
		case SPT_VEC2:		bytes = sizeof(Vector2)*m_count;	break;
		case SPT_VEC3:		bytes = sizeof(Vector3)*m_count;	break;
		case SPT_VEC4:		bytes = sizeof(Vector4)*m_count;	break;
		case SPT_MAT4:		bytes = sizeof(Matrix4)*m_count;	break;
		case SPT_TEXTURE:	bytes = sizeof(int)*m_count;		break;
		default:			bytes = 0;							break;
		}

		return bytes;
	}

	void Material::Uniform::setValue(const void* value)
	{
		if (value)
		{
			i32 bytes = getValueBytes();
			if (!m_value)
				m_value = EchoNewArray(Byte, bytes);

			std::memcpy(m_value, value, bytes);
		}
	}

	void Material::Uniform::allocValue()
	{
		if (!m_value)
		{
			i32 bytes = getValueBytes();
			m_value = EchoNewArray(Byte, bytes);
			std::memset(m_value, 0, bytes);
		}
	}

	Material::Material()
		: Res()
		, m_isDirty(false)
		, m_shaderPath("", ".shader")
		, m_renderStage("Opaque", { "Opaque", "Transparent" })
	{
	}

	Material::Material(const ResourcePath& path)
		: Res(path)
		, m_isDirty(false)
		, m_shaderPath("", ".shader")
		, m_renderStage("Opaque", { "Opaque", "Transparent" })
	{
	}

	Material::~Material()
	{
		EchoSafeDeleteMap(m_uniforms, Uniform);

		unloadTexture();
	}

	// bind methods to script
	void Material::bindMethods()
	{
		CLASS_BIND_METHOD(Material, getShaderPath, DEF_METHOD("getShaderPath"));
		CLASS_BIND_METHOD(Material, setShaderPath, DEF_METHOD("setShaderPath"));
		CLASS_BIND_METHOD(Material, setRenderStage, DEF_METHOD("setRenderStage"));
		CLASS_BIND_METHOD(Material, getRenderStage, DEF_METHOD("getRenderStage"));

		CLASS_REGISTER_PROPERTY(Material, "Shader", Variant::Type::ResourcePath, "getShaderPath", "setShaderPath");
		CLASS_REGISTER_PROPERTY(Material, "Stage", Variant::Type::StringOption, "getRenderStage", "setRenderStage");
	}

	void Material::clone(Material* orig)
	{
		m_renderStage = orig->m_renderStage;
		m_macros = orig->m_macros;
		m_shaderProgram = orig->m_shaderProgram;
        
		for (auto it : orig->m_uniforms)
		{
			Uniform* uniform = it.second;
			m_uniforms[uniform->m_name] = uniform->clone();
		}

		m_textures = orig->m_textures;
	}

	void* Material::getUniformValue(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			return  it->second->m_value;
		}

		const ShaderProgram::UniformValue* dUniform = m_shaderProgram->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : nullptr;
	}

	void Material::loadTexture()
	{
		for (const auto& element : m_textures)
		{
			m_textures[element.first].m_texture = (Texture*)Res::get(element.second.m_uri);
		}
	}

	void Material::unloadTexture()
	{
		m_textures.clear();
	}

	Texture* Material::getTexture(const int& index)
	{
		auto it = m_textures.find(index);
		if (it != m_textures.end())
		{
			return it->second.m_texture;
		}

		return nullptr;
	}

	const String& Material::getTexturePath(const int& index)
	{
		auto it = m_textures.find(index);
		if (it != m_textures.end())
		{
			return it->second.m_uri;
		}

		static String blank = "";
		return blank;
	}

	void Material::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());

		m_isDirty = true;
	}

	ShaderProgram* Material::getShader()
	{ 
		buildShaderProgram();

		return m_shaderProgram;
	}

	bool Material::isUniformExist(const String& name)
	{
		return m_uniforms.find(name)!=m_uniforms.end();
	}

	void Material::setUniformValue(const String& name, const ShaderParamType& type, const void* value)
	{
		buildShaderProgram();

		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			if (it->second->m_type == type)
				it->second->setValue(value);
			else
				EchoLogError("MaterialInstance::ModifyUnifromParam Type Error!");
		}
	}

	bool Material::isGlobalUniform(const String& name)
	{
		if (name == "u_WVPMaterix")
			return true;

		return false;
	}

	void Material::addTexture(int idx, const String& name)
	{
		TextureInfo info;
		info.m_idx = idx;
		info.m_name = name;
		m_textures[idx] = info;
	}

	Texture* Material::setTexture(const String& name, const String& uri)
	{
		return setTexture( name, (Texture*)Res::get(uri));
	}

	Texture* Material::setTexture(const String& name, TexturePtr texture)
	{
		buildShaderProgram();

		if (!texture)
			return nullptr;

		for (auto& it : m_textures)
		{
			TextureInfo& info = it.second;
			if (info.m_name == name)
			{
				info.m_uri = texture->getPath();
				info.m_texture = texture;

				return info.m_texture;
			}
		}

		return nullptr;
	}

	Material::Uniform* Material::getUniform(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
			return it->second;

		return NULL;
	}

	bool Material::isMacroUsed(const String& macro)
	{
		for (const String& _macro : m_macros)
		{
			if (macro == _macro)
				return true;
		}

		return false;
	}

	void Material::setMacro(const String& macro, bool enabled)
	{
		if (enabled)
		{
			if (!isMacroUsed(macro))
			{
				m_macros.push_back(macro);
			}
		}
		else
		{
            m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), macro), m_macros.end());
		}
        
		std::sort(m_macros.begin(), m_macros.end());

		m_isDirty = true;
	}

	void Material::buildShaderProgram()
	{
		if (m_isDirty)
		{
			clearPropertys();

			// create material
			m_shaderProgram = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(m_shaderPath));
			if(!m_shaderProgram)
			{
				m_shaderProgram = (ShaderProgram*)ShaderProgram::create();
                m_shaderProgram->setMacros(m_macros);
                m_shaderProgram->load(m_shaderPath);
			}

			// match uniforms
			if (m_shaderProgram)
			{
                matchUniforms();
			}

			// register uniform propertys
			if (m_shaderProgram)
			{
				StringArray macros = ShaderProgram::getEditableMacros();
				for (size_t i = 0; i < macros.size() / 2; i++)
				{
					registerProperty(ECHO_CLASS_NAME(Material), "Macros." + macros[i * 2], Variant::Type::Bool);
				}

				for (auto& it : m_uniforms)
				{
					if (!isGlobalUniform(it.first))
					{
						switch (it.second->m_type)
						{
						case ShaderParamType::SPT_INT: registerProperty(ECHO_CLASS_NAME(Material), "Uniforms." + it.first, Variant::Type::Int); break;
						case ShaderParamType::SPT_FLOAT:registerProperty(ECHO_CLASS_NAME(Material), "Uniforms." + it.first, Variant::Type::Real); break;
						case ShaderParamType::SPT_VEC3: registerProperty(ECHO_CLASS_NAME(Material), "Uniforms." + it.first, Variant::Type::Vector3); break;
						case ShaderParamType::SPT_VEC4: registerProperty(ECHO_CLASS_NAME(Material), "Uniforms." + it.first, Variant::Type::Color); break;
						case ShaderParamType::SPT_TEXTURE: registerProperty(ECHO_CLASS_NAME(Material), "Uniforms." + it.first, Variant::Type::ResourcePath); break;
						default: break;
						}
					}
				}
			}

			m_isDirty = false;
		}
	}

	const PropertyInfos& Material::getPropertys()
	{
		buildShaderProgram();

		return m_propertys;
	}

	bool Material::getPropertyValue(const String& propertyName, Variant& oVar) 
	{ 
		buildShaderProgram();

		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			Uniform* uniform = getUniform(ops[1]);
			switch (uniform->m_type)
			{
			case ShaderParamType::SPT_FLOAT:	oVar = *(float*)(uniform->m_value); break;
			case ShaderParamType::SPT_VEC2:		oVar = *(Vector2*)(uniform->m_value); break;
			case ShaderParamType::SPT_VEC3:		oVar = *(Vector3*)(uniform->m_value); break;
			case ShaderParamType::SPT_VEC4:		oVar = *(Color*)(uniform->m_value); break;
			case ShaderParamType::SPT_TEXTURE : oVar = ResourcePath(getTexturePath(*(int*)uniform->m_value), ".png"); break;
			default:							oVar = *(float*)(uniform->m_value); break;
			}
		}
		else if (ops[0] == "Macros")
		{
			oVar = isMacroUsed(ops[1]);
		}

		return false; 
	}

	// set property value
	bool Material::setPropertyValue(const String& propertyName, const Variant& propertyValue)
	{
		buildShaderProgram();

		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			Uniform* uniform = getUniform(ops[1]);
			switch (uniform->m_type)
			{
			case ShaderParamType::SPT_FLOAT:	setUniformValue(ops[1], uniform->m_type, &(propertyValue.toReal())); break;
			case ShaderParamType::SPT_VEC2:		setUniformValue(ops[1], uniform->m_type, &(propertyValue.toVector2())); break;
			case ShaderParamType::SPT_VEC3:		setUniformValue(ops[1], uniform->m_type, &(propertyValue.toVector3())); break;
			case ShaderParamType::SPT_VEC4:		setUniformValue(ops[1], uniform->m_type, &(propertyValue.toColor())); break;
			case ShaderParamType::SPT_TEXTURE:  setTexture(ops[1], propertyValue.toResPath().getPath()); break;
			default:							setUniformValue(ops[1], uniform->m_type, &(propertyValue.toReal())); break;
			}
		}
		else if (ops[0] == "Macros")
		{
			setMacro(ops[1], propertyValue);
		}

		return false;
	}

	void Material::matchUniforms()
	{
		if (m_shaderProgram)
		{
			for (auto& it : *(m_shaderProgram->getUniforms()))
			{
				const ShaderProgram::Uniform& suniform = it.second;
				{
					Uniform* uniform = EchoNew(Uniform);
					uniform->m_name = suniform.m_name;
					uniform->m_type = suniform.m_type;
					uniform->m_count = suniform.m_count;

					// auto set texture value
					if (uniform->m_type == SPT_TEXTURE)
					{
						i32 textureNum = getTextureNum();
						uniform->setValue(&textureNum);
						addTexture(getTextureNum(), uniform->m_name);
					}
					else
					{
						uniform->allocValue();
					}

					// default value
					const ShaderProgram::UniformValue* defaultUniform = m_shaderProgram->getDefaultUniformValue(uniform->m_name);
					if (defaultUniform && uniform->m_count == defaultUniform->count && uniform->m_type == defaultUniform->type)
						uniform->setValue(defaultUniform->value);

					m_uniforms[uniform->m_name] = uniform;
				}
			}
		}
	}

	void Material::setShaderPath(const ResourcePath& path) 
	{ 
		m_shaderPath = path;

		m_isDirty = true;
	}
}
