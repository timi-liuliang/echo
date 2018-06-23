#include "Material.h"
#include "Engine/modules/Light/Light.h"
#include "Engine/core/Scene/NodeTree.h"
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
		EchoSafeDeleteArray( m_value, Byte, getValueBytes());
		if (value)
		{
			i32 bytes = getValueBytes();
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
		: Res(ResourcePath("", ".material"))
		, m_shaderPath("", ".shader")
		, m_shaderProgram(NULL)
		, m_officialShaderContent(nullptr)
		, m_renderStage("", nullptr)
	{
		m_renderStage.addOption("Opaque");
		m_renderStage.addOption("Transparent");
		m_renderStage.setValue("Opaque");
	}

	// 构造函数
	Material::Material(const ResourcePath& path)
		: Res(path)
		, m_shaderPath("", ".shader")
		, m_shaderProgram(NULL)
		, m_officialShaderContent(nullptr)
		, m_renderStage("", nullptr)
	{
		m_renderStage.addOption("Opaque");
		m_renderStage.addOption("Transparent");
		m_renderStage.setValue("Opaque");
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
		CLASS_BIND_METHOD(Material, getShader, DEF_METHOD("getShader"));
		CLASS_BIND_METHOD(Material, setShader, DEF_METHOD("setShader"));
		CLASS_BIND_METHOD(Material, setRenderStage, DEF_METHOD("setRenderStage"));
		CLASS_BIND_METHOD(Material, getRenderStage, DEF_METHOD("getRenderStage"));

		CLASS_REGISTER_PROPERTY(Material, "Shader", Variant::Type::ResourcePath, "getShader", "setShader");
		CLASS_REGISTER_PROPERTY(Material, "Stage", Variant::Type::StringOption, "getRenderStage", "setRenderStage");
	}

	// release
	void Material::release()
	{
		ECHO_DELETE_T(this, Material);
	}

	bool Material::applyLoadedData()
	{
		// 加载材质模板
		buildRenderQueue();

		// 获取着色器
		ShaderProgram* shaderProgram = m_shaderProgram->getShaderProgram();
		if (shaderProgram)
		{
			matchUniforms();
			return true;
		}

		return false;
	}

	// 复制材质实例
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

	// 获取变量值
	void* Material::getUniformValue(const String& name)
	{
		const auto& it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			return  it->second->m_value;
		}

		const ShaderProgramRes::DefaultUniform* dUniform = m_shaderProgram->getDefaultUniformValue(name);
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

	// 设置宏定义
	void Material::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());
	}

	// 判断变量是否存在
	bool Material::isUniformExist(const String& name)
	{
		return m_uniforms.find(name)!=m_uniforms.end();
	}

	// 修改变量
	void Material::setUniformValue(const String& name, const ShaderParamType& type, void* value)
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
		if (!textureRes)
			return nullptr;

		for (auto& it : m_textures)
		{
			TextureInfo& info = it.second;
			if (info.m_name == name)
			{
				//TextureRes::releaseResource(info.m_texture);

				info.m_uri = textureRes->getName();
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
	}

	// 构建渲染队列
	void Material::buildRenderQueue()
	{
		// make sure macros
		String finalMacros; finalMacros.reserve(512);
		for (const String& macro : m_macros)
		{
			finalMacros += "#define " + macro + "\n";
		}

		// create material
		m_shaderProgram = EchoNew(ShaderProgramRes);
		if (m_officialShaderContent)
			m_shaderProgram->loadFromContent(m_officialShaderContent, finalMacros);
		else if (!m_shaderPath.getPath().empty())
			m_shaderProgram->loadFromFile( m_shaderPath.getPath(), finalMacros);
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
		ShaderProgram* shaderProgram = m_shaderProgram->getShaderProgram();
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
					const ShaderProgramRes::DefaultUniform* defaultUniform = m_shaderProgram->getDefaultUniformValue(uniform->m_name);
					if (defaultUniform && uniform->m_count == defaultUniform->count && uniform->m_type == defaultUniform->type)
						uniform->setValue(defaultUniform->value);

					m_uniforms[uniform->m_name] = uniform;
				}
			}
		}
	}

	void Material::setShader(const ResourcePath& path) 
	{ 
		m_shaderPath = path; 
	}
}