#include "engine/core/log/Log.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/base/ShaderProgram.h"
#include "engine/core/render/base/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	Material::Uniform::Uniform(const String& name, ShaderParamType type, i32 count)
		: m_name(name)
		, m_type(type)
		, m_count(count)
	{
		m_value.resize(getValueBytes(), 0);
	}

	Material::Uniform::~Uniform()
	{
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
			std::memcpy(m_value.data(), value, bytes);
		}
	}

	Material::Material()
		: Res()
		, m_shaderPath("", ".shader")
	{
	}

	Material::Material(const ResourcePath& path)
		: Res(path)
		, m_shaderPath("", ".shader")
	{
	}

	Material::~Material()
	{
		EchoSafeDeleteMap(m_uniforms, Uniform);

		unloadTexture();
	}

	void Material::bindMethods()
	{
		CLASS_BIND_METHOD(Material, getShaderPath, DEF_METHOD("getShaderPath"));
		CLASS_BIND_METHOD(Material, setShaderPath, DEF_METHOD("setShaderPath"));
		CLASS_BIND_METHOD(Material, getRenderStage, DEF_METHOD("getRenderStage"));

		CLASS_REGISTER_PROPERTY(Material, "Shader", Variant::Type::ResourcePath, "getShaderPath", "setShaderPath");
	}

	void* Material::getUniformValue(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			return  it->second->m_value.data();
		}

		const ShaderProgram::UniformValue* dUniform = m_shaderProgram->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : nullptr;
	}

	void Material::loadTexture()
	{
		for (auto& info : m_textures)
		{
			info.m_texture = (Texture*)(Res::get(info.m_uri));
		}
	}

	void Material::unloadTexture()
	{
		m_textures.clear();
	}

	Texture* Material::getTexture(const int& index)
	{
		if (index < m_textures.size())
		{
			return m_textures[index].m_texture;
		}

		return nullptr;
	}

	const String& Material::getTexturePath(const int& index)
	{
		if (index < m_textures.size())
		{
			return m_textures[index].m_uri;
		}

		static String blank = "";
		return blank;
	}

	void Material::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());

		buildShaderProgram();
	}

	ShaderProgram* Material::getShader()
	{ 
		return m_shaderProgram;
	}

	bool Material::isUniformExist(const String& name)
	{
		return m_uniforms.find(name)!=m_uniforms.end();
	}

	void Material::setUniformValue(const String& name, const ShaderParamType& type, const void* value)
	{
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

	void Material::addTexture(const String& name)
	{
		TextureInfo info;
		info.m_name = name;
		m_textures.push_back(info);
	}

	Texture* Material::setTexture(const String& name, const String& uri)
	{
		return setTexture( name, (Texture*)Res::get(uri));
	}

	Texture* Material::setTexture(const String& name, TexturePtr texture)
	{
		if (!texture)
			return nullptr;

		for (TextureInfo& info : m_textures)
		{
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

	void Material::setShaderPath(const ResourcePath& path)
	{
		m_shaderPath = path;

		buildShaderProgram();

		if (m_shaderProgram)
		{
			m_shaderProgram->onShaderChanged.connectClassMethod(this, createMethodBind(&Material::buildShaderProgram));
		}
	}

	const String& Material::getRenderStage()
	{ 
		return m_shaderProgram ? m_shaderProgram->getBlendMode().getValue() : StringUtil::BLANK;
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

		buildShaderProgram();
	}

	void Material::buildShaderProgram()
	{
		if (!m_shaderPath.isEmpty())
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

			// emit signal
			onShaderChanged();
		}
	}

	const PropertyInfos& Material::getPropertys()
	{
		return m_propertys;
	}

	bool Material::getPropertyValue(const String& propertyName, Variant& oVar) 
	{ 
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			Uniform* uniform = getUniform(ops[1]);
			switch (uniform->m_type)
			{
			case ShaderParamType::SPT_FLOAT:	oVar = *(float*)(uniform->m_value.data()); break;
			case ShaderParamType::SPT_VEC2:		oVar = *(Vector2*)(uniform->m_value.data()); break;
			case ShaderParamType::SPT_VEC3:		oVar = *(Vector3*)(uniform->m_value.data()); break;
			case ShaderParamType::SPT_VEC4:		oVar = *(Color*)(uniform->m_value.data()); break;
			case ShaderParamType::SPT_TEXTURE : oVar = ResourcePath(getTexturePath(*(int*)uniform->m_value.data()), ".png"); break;
			default:							oVar = *(float*)(uniform->m_value.data()); break;
			}
		}
		else if (ops[0] == "Macros")
		{
			oVar = isMacroUsed(ops[1]);
		}

		return false; 
	}

	bool Material::setPropertyValue(const String& propertyName, const Variant& propertyValue)
	{
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
			ParamMap oldUniforms = m_uniforms;
			TextureInfoArray oldTextureInfos = std::move(m_textures);

			m_uniforms.clear();
			m_textures.clear();

			for (auto& it : *(m_shaderProgram->getUniforms()))
			{
				const ShaderProgram::Uniform& suniform = it.second;
				{
					Uniform* uniform = EchoNew(Uniform(suniform.m_name, suniform.m_type, suniform.m_count));

					// auto set texture value
					if (uniform->m_type == SPT_TEXTURE)
					{
						i32 textureNum = getTextureNum();
						uniform->setValue(&textureNum);
						addTexture(uniform->m_name);

						// use old values
						for (TextureInfo& info : oldTextureInfos)
						{
							if (info.m_name == uniform->m_name)
							{
								setTexture(uniform->m_name, info.m_texture);
								break;
							}
						}
					}
					else
					{
						// old value
						auto it = oldUniforms.find(uniform->m_name);
						if (it != oldUniforms.end())
						{
							Uniform* oldUniform = it->second;
							if (oldUniform && uniform->m_count == oldUniform->m_count && uniform->m_type == oldUniform->m_type)
							{
								uniform->setValue(oldUniform->m_value.data());
							}
						}
						else
						{
							// default value
							const ShaderProgram::UniformValue* defaultUniform = m_shaderProgram->getDefaultUniformValue(uniform->m_name);
							if (defaultUniform && uniform->m_count == defaultUniform->count && uniform->m_type == defaultUniform->type)
							{
								uniform->setValue(defaultUniform->value);
							}
						}
					}

					m_uniforms[uniform->m_name] = uniform;
				}
			}
		}
	}
}
