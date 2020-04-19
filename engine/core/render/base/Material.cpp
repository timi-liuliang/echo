#include "engine/core/log/Log.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/base/ShaderProgram.h"
#include "engine/core/render/base/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	Material::UniformNormalValue::UniformNormalValue(const ShaderProgram::UniformPtr uniform)
	{
		m_uniform = uniform;
	}

	const void* Material::UniformNormalValue::getValue()
	{
		return m_value.empty() ? m_uniform->getValueDefault().data() : m_value.data();
	}

	void Material::UniformNormalValue::setValue(const void* value)
	{
		if (value)
		{
			m_value.resize(m_uniform->m_sizeInBytes, 0);
			std::memcpy(m_value.data(), value, m_uniform->m_sizeInBytes);

			// clear
			if (m_value == m_uniform->getValueDefault())
				m_value.clear();
		}
		else
		{
			m_value.clear();
		}
	}

	Material::UniformTextureValue::UniformTextureValue(const ShaderProgram::UniformPtr uniform, Material* owner)
		: m_owner(owner)
	{
		m_uniform = uniform;
	}

	ResourcePath Material::UniformTextureValue::getTexturePath()
	{ 
		return m_uri.getPath().empty() ? m_uniform->getTextureDefault() : m_uri; 
	}

	Texture* Material::UniformTextureValue::getTexture()
	{
		if (!m_texture)
		{
			ResourcePath path = m_uri.isEmpty() ? m_uniform->getTextureDefault() : m_uri;
			if (Echo::PathUtil::GetFileExt(path.getPath(), false) == "atla")
			{
				m_atla = (TextureAtla*)Res::get(path);
				m_texture = m_atla->getTexture();

				Material::UniformValue* viewportValue = m_owner->getUniform(m_uniform->m_name + "Viewport");
				if (viewportValue)
				{
					viewportValue->setValue(&m_atla->getViewportNormalized());
				}
			}
			else
			{
				m_texture = (Texture*)Res::get(path);
			}
		}

		return m_texture; 
	}

	Texture* Material::UniformTextureValue::setTexture(const String& uri)
	{
		if (uri == m_uniform->getTextureDefault().getPath())
		{
			m_uri.clear();
		}
		else
		{
			m_uri = uri;
		}

		m_texture = nullptr;
		return getTexture();
	}

	Texture* Material::UniformTextureValue::setTexture(TexturePtr texture)
	{
		m_texture = texture;
		return m_texture;
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
		EchoSafeDeleteMap(m_uniformValues, UniformValue);
	}

	void Material::bindMethods()
	{
		CLASS_BIND_METHOD(Material, getShaderPath, DEF_METHOD("getShaderPath"));
		CLASS_BIND_METHOD(Material, setShaderPath, DEF_METHOD("setShaderPath"));
		CLASS_BIND_METHOD(Material, getRenderStage, DEF_METHOD("getRenderStage"));

		CLASS_REGISTER_PROPERTY(Material, "Shader", Variant::Type::ResourcePath, "getShaderPath", "setShaderPath");
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
		return m_uniformValues.find(name)!=m_uniformValues.end();
	}

	Material::UniformValue* Material::getUniform(const String& name)
	{
		const auto& it = m_uniformValues.find(name);
		if (it != m_uniformValues.end())
			return it->second;

		return NULL;
	}

	void Material::setShaderPath(const ResourcePath& path)
	{
		if (m_shaderPath.setPath(path.getPath(), true))
		{
			buildShaderProgram();

			if (m_shaderProgram)
			{
				m_shaderProgram->onShaderChanged.connectClassMethod(this, createMethodBind(&Material::buildShaderProgram));
			}
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
			m_shaderProgram = ShaderProgram::getDefault(m_shaderPath.getPath());
			if(!m_shaderProgram)
			{
				m_shaderProgram = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(m_shaderPath));
				if (!m_shaderProgram)
				{
					m_shaderProgram = (ShaderProgram*)ShaderProgram::create();
					m_shaderProgram->setMacros(m_macros);
					m_shaderProgram->load(m_shaderPath);
				}
			}

			// match uniforms
			if (m_shaderProgram)
			{
                matchUniforms();
			}

			// register uniform properties
			if (m_shaderProgram)
			{
				StringArray macros = ShaderProgram::getEditableMacros();
				for (size_t i = 0; i < macros.size() / 2; i++)
				{
					registerProperty(ECHO_CLASS_NAME(Material), "Macros." + macros[i * 2], Variant::Type::Bool);
				}

				for (auto& it : m_uniformValues)
				{
					if (!ShaderProgram::isGlobalUniform(it.first))
					{
						switch (it.second->m_uniform->m_type)
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

	bool Material::getPropertyValue(const String& propertyName, Variant& oVar) 
	{ 
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			UniformValue* uniform = getUniform(ops[1]);
			switch (uniform->m_uniform->m_type)
			{
			case ShaderParamType::SPT_FLOAT:	oVar = *(float*)(uniform->getValue()); break;
			case ShaderParamType::SPT_VEC2:		oVar = *(Vector2*)(uniform->getValue()); break;
			case ShaderParamType::SPT_VEC3:		oVar = *(Vector3*)(uniform->getValue()); break;
			case ShaderParamType::SPT_VEC4:		oVar = *(Color*)(uniform->getValue()); break;
			case ShaderParamType::SPT_TEXTURE : oVar = uniform->getTexturePath(); break;
			default:							oVar = *(float*)(uniform->getValue()); break;
			}
		}
		else if (ops[0] == "Macros")
		{
			oVar = isMacroUsed(ops[1]);
		}

		return false; 
	}

	bool Material::getPropertyValueDefault(const String& propertyName, Variant& oVar)
	{
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			ShaderProgram::UniformPtr uniform = getUniform(ops[1])->m_uniform;
			switch (uniform->m_type)
			{
			case ShaderParamType::SPT_FLOAT:	oVar = *(float*)(uniform->getValueDefault().data()); break;
			case ShaderParamType::SPT_VEC2:		oVar = *(Vector2*)(uniform->getValueDefault().data()); break;
			case ShaderParamType::SPT_VEC3:		oVar = *(Vector3*)(uniform->getValueDefault().data()); break;
			case ShaderParamType::SPT_VEC4:		oVar = *(Color*)(uniform->getValueDefault().data()); break;
			case ShaderParamType::SPT_TEXTURE:  oVar = uniform->getTextureDefault(); break;
			default:							oVar = *(float*)(uniform->getValueDefault().data()); break;
			}
		}
		else if (ops[0] == "Macros")
		{
			oVar = isMacroUsed(ops[1]);
		}

		return true;
	}

	i32 Material::getPropertyFlag(const String& propertyName)
	{
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			UniformValue* uniformValue = getUniform(ops[1]);
			if (uniformValue->isEmpty())
				return 0;
		}

		return PropertyFlag::All;
	}

	bool Material::setPropertyValue(const String& propertyName, const Variant& propertyValue)
	{
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			UniformValue* uniform = getUniform(ops[1]);
			if (uniform)
			{
				ShaderParamType uniformType = uniform->m_uniform->m_type;
				switch (uniformType)
				{
				case ShaderParamType::SPT_FLOAT:	uniform->setValue(&(propertyValue.toReal())); break;
				case ShaderParamType::SPT_VEC2:		uniform->setValue(&(propertyValue.toVector2())); break;
				case ShaderParamType::SPT_VEC3:		uniform->setValue(&(propertyValue.toVector3())); break;
				case ShaderParamType::SPT_VEC4:		uniform->setValue(&(propertyValue.toColor())); break;
				case ShaderParamType::SPT_TEXTURE:  uniform->setTexture(propertyValue.toResPath().getPath()); break;
				default:							uniform->setValue(&(propertyValue.toReal())); break;
				}
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
			UniformValueMap oldUniforms = m_uniformValues;
			m_uniformValues.clear();

			for (auto& it : m_shaderProgram->getUniforms())
			{
				// create
				ShaderProgram::UniformPtr& suniform = it.second;
				UniformValue* uniform = nullptr;
				if (suniform->m_type == SPT_TEXTURE)
					uniform = EchoNew(UniformTextureValue(suniform, this));
				else
					uniform = EchoNew(UniformNormalValue(suniform));

				// copy data
				UniformValueMap::iterator it = oldUniforms.find(suniform->m_name);
				if (it != oldUniforms.end())
				{
					UniformValue* oldUniform = it->second;
					if (oldUniform && suniform->m_count == oldUniform->m_uniform->m_count && suniform->m_type == oldUniform->m_uniform->m_type)
					{
						if (suniform->m_type == SPT_TEXTURE)
							uniform->setTexture(oldUniform->getTexturePath().getPath());
						else
							uniform->setValue(oldUniform->isEmpty() ? nullptr : oldUniform->getValue());
					}
				}

				m_uniformValues[suniform->m_name] = uniform;
			}
		}
	}
}
