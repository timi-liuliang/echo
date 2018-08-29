#include "engine/core/log/Log.h"
#include "Material.h"
#include "engine/modules/light/Light.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/render/ShaderProgramRes.h"
#include "engine/core/render/render/MaterialDesc.h"
#include "engine/core/render/render/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	Material::Uniform::~Uniform()
	{
		EchoSafeDeleteArray(m_value, Byte, getValueBytes());
	}

	// 克隆
	Material::Uniform* Material::Uniform::clone()
	{
		Uniform* result = EchoNew(Uniform);
		result->m_name = m_name;
		result->m_type = m_type;
		result->m_count = m_count;
		result->setValue( m_value);

		return result;
	}

	// get value bytes
	ui32 Material::Uniform::getValueBytes()
	{
		// 计算所需内存大小
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

	// alloc Value
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
		, m_shaderContent(nullptr)
		, m_renderStage("Opaque", { "Opaque", "Transparent" })
        , m_shaderProgramRes(nullptr)
	{
	}

	// 构造函数
	Material::Material(const ResourcePath& path)
		: Res(path)
		, m_isDirty(false)
		, m_shaderPath("", ".shader")
		, m_shaderContent(nullptr)
		, m_renderStage("Opaque", { "Opaque", "Transparent" })
        , m_shaderProgramRes(nullptr)
	{
	}

	// 析构函数
	Material::~Material()
	{
		m_uniforms.clear();

		unloadTexture();
		m_textures.clear();
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

	// release
	void Material::release()
	{
		ECHO_DELETE_T(this, Material);
	}

	// 复制材质实例
	void Material::clone(Material* orig)
	{
		m_renderStage = orig->m_renderStage;
		m_macros = orig->m_macros;
		m_shaderProgramRes = orig->m_shaderProgramRes;

		for (auto it : orig->m_uniforms)
		{
			Uniform* uniform = it.second;
			m_uniforms[uniform->m_name] = uniform->clone();
		}

		m_textures = orig->m_textures;
	}

	// 获取变量值
	void* Material::getUniformValue(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			return  it->second->m_value;
		}

		const ShaderProgramRes::DefaultUniform* dUniform = m_shaderProgramRes->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : NULL;
	}

	// 准备资源IO
	TextureRes* Material::prepareTextureImp(const String& texName)
	{
		TextureRes* pTexture;
		size_t cubePos = texName.find("_cube_");
		if (texName.find("_cube_") == String::npos)
		{
			pTexture = TextureRes::createTexture(texName.empty() ? "OFFICAL_MATERTAL_TEMPLATE.tga" : texName, Texture::TU_STATIC);
		}
		else
		{
			String cubeTexNamePre = texName.substr(0, cubePos + 6);
			String cubeTexNamePost = texName.substr(cubePos + 7);
			array<String, 6> texNames;
			for (size_t i = 0; i < 6; ++i)
			{
				texNames[i] = StringUtil::Format("%s%d%s", cubeTexNamePre.c_str(), i, cubeTexNamePost.c_str());
			}
			pTexture = TextureRes::createTextureCubeFromFiles(texNames[0], texNames[1], texNames[2], texNames[3], texNames[4], texNames[5]);
		}

		pTexture->prepareLoad();

		return pTexture;
	}

	// 资源加载线程准备纹理
	void Material::prepareTexture()
	{
		for (const auto& element : m_textures)
		{
			m_textures[element.first].m_texture = prepareTextureImp(element.second.m_uri);
		}
	}

	void Material::loadTexture()
	{
		for (auto& it : m_textures)
		{
			TextureRes* texRes = it.second.m_texture;
			texRes->prepareLoad();
		}
	}

	void Material::unloadTexture()
	{
		m_textures.clear();
	}

	// 根据索引获取纹理
	TextureRes* Material::getTexture(const int& index)
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

	// 设置宏定义
	void Material::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());

		m_isDirty = true;
	}

	// 获取渲染队列
	ShaderProgramRes* Material::getShader() 
	{ 
		buildShaderProgram();

		return m_shaderProgramRes; 
	}

	// 判断变量是否存在
	bool Material::isUniformExist(const String& name)
	{
		return m_uniforms.find(name)!=m_uniforms.end();
	}

	// 修改变量
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

	// is global uniform
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

	TextureRes* Material::setTexture(const String& name, const String& uri)
	{
		return setTexture( name, prepareTextureImp(uri));
	}

	TextureRes* Material::setTexture(const String& name, TextureRes* textureRes)
	{
		buildShaderProgram();

		if (!textureRes)
			return nullptr;

		for (auto& it : m_textures)
		{
			TextureInfo& info = it.second;
			if (info.m_name == name)
			{
				info.m_uri = textureRes->getPath();
				info.m_texture = textureRes;

				return info.m_texture;
			}
		}

		return nullptr;
	}

	// 获取变量
	Material::Uniform* Material::getUniform(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
			return it->second;

		return NULL;
	}

	// 是否使用了宏定义
	bool Material::isMacroUsed(const String& macro)
	{
		for (const String& _macro : m_macros)
		{
			if (macro == _macro)
				return true;
		}

		return false;
	}

	// 设置宏定义
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
			if (isMacroUsed(macro))
			{
				m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), macro), m_macros.end());
			}
			else
			{
				ui32 size = m_macros.size();
				for (ui32 i = 0; i < size; ++i)
				{
					if (StringUtil::StartWith(m_macros[i], macro))
					{
						m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), m_macros[i]), m_macros.end());
						break;
					}
				}
			}
		}
		std::sort(m_macros.begin(), m_macros.end());

		m_isDirty = true;
	}

	// 构建渲染队列
	void Material::buildShaderProgram()
	{
		if (m_isDirty)
		{
			clearPropertys();

			// make sure macros
			String finalMacros; finalMacros.reserve(512);
			for (const String& macro : m_macros)
				finalMacros += "#define " + macro + "\n";

			// create material
			m_shaderProgramRes = EchoNew(ShaderProgramRes);
			if (m_shaderContent)
				m_shaderProgramRes->loadFromContent(m_shaderContent, finalMacros);
			else if (!m_shaderPath.getPath().empty())
				m_shaderProgramRes->loadFromFile(m_shaderPath.getPath(), finalMacros);

			// match uniforms
			if (m_shaderProgramRes)
			{
				ShaderProgram* shaderProgram = m_shaderProgramRes->getShaderProgram();
				if (shaderProgram)
					matchUniforms();
			}

			// register uniform propertys
			if (m_shaderProgramRes)
			{
				StringArray macros = ShaderProgramRes::getEditableMacros(m_shaderPath.getPath());
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

	// propertys (script property or dynamic property)
	const PropertyInfos& Material::getPropertys()
	{
		buildShaderProgram();

		return m_propertys;
	}

	// get property value
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

	static bool MappingStringArrayIdx(const String* arry, int count, const String& value, int& idx)
	{
		for (int i = 0; i < count; i++)
		{
			if (value == arry[i])
			{
				idx = i;
				return true;
			}
		}

		return false;
	}

	// 参数匹配
	void Material::matchUniforms()
	{
		ShaderProgram* shaderProgram = m_shaderProgramRes->getShaderProgram();
		if (shaderProgram)
		{
			// 添加未设置参数
			for (auto& it : *(shaderProgram->getUniforms()))
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
					const ShaderProgramRes::DefaultUniform* defaultUniform = m_shaderProgramRes->getDefaultUniformValue(uniform->m_name);
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

	// 设置使用官方材质
	void Material::setShaderContent(const char* content)
	{
		m_shaderContent = content;

		m_isDirty = true;
	}
}
