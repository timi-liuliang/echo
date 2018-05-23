#include "MaterialInst.h"
#include "Engine/modules/Light/Light.h"
#include "Engine/core/Scene/NodeTree.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/MaterialDesc.h"
#include "engine/core/render/render/Renderer.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	MaterialInst::Uniform::~Uniform()
	{
		EchoSafeDeleteArray(m_value, Byte, getValueBytes());
	}

	// 克隆
	MaterialInst::Uniform* MaterialInst::Uniform::clone()
	{
		Uniform* result = EchoNew(Uniform);
		result->m_name = m_name;
		result->m_type = m_type;
		result->m_count = m_count;
		result->setValue( m_value);

		return result;
	}

	// get value bytes
	i32 MaterialInst::Uniform::getValueBytes()
	{
		// 计算所需内存大小
		int bytes = 0;
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

	void MaterialInst::Uniform::setValue(const void* value)
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
	void MaterialInst::Uniform::allocValue()
	{
		if (!m_value)
		{
			i32 bytes = getValueBytes();
			m_value = EchoNewArray(Byte, bytes);
			std::memset(m_value, 0, bytes);
		}
	}

	// 构造函数
	MaterialInst::MaterialInst()
		: m_material(NULL)
		, m_officialMaterialContent(nullptr)
	{

	}

	// 析构函数
	MaterialInst::~MaterialInst()
	{
		m_unifroms.clear();

		unloadTexture();
		m_textures.clear();
	}

	// create a material instance
	MaterialInst* MaterialInst::create()
	{
		return EchoNew(MaterialInst);
	}

	// release
	void MaterialInst::release()
	{
		ECHO_DELETE_T(this, MaterialInst);
	}

	bool MaterialInst::applyLoadedData()
	{
		// 加载材质模板
		buildRenderQueue();

		// 获取着色器
		ShaderProgram* shaderProgram = m_material->getShaderProgram();
		if (shaderProgram)
		{
			matchUniforms();
			return true;
		}

		return false;
	}

	// 复制材质实例
	void MaterialInst::clone(MaterialInst* orig)
	{
		// 拷贝名称，材质
		m_name = orig->m_name;
		m_renderStage = orig->m_renderStage;
		m_macros = orig->m_macros;
		m_materialTemplate = orig->m_materialTemplate;
		m_material = orig->m_material;

		for (auto it : orig->m_unifroms)
		{
			Uniform* uniform = it.second;
			m_unifroms[uniform->m_name] = uniform->clone();
		}

		m_textures = orig->m_textures;
	}

	// 获取变量值
	void* MaterialInst::getUniformValue(const String& name)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
		{
			return  it->second->m_value;
		}

		const Material::DefaultUniform* dUniform = m_material->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : NULL;
	}

	// 准备资源IO
	TextureRes* MaterialInst::prepareTextureImp(const String& texName)
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
	void MaterialInst::prepareTexture()
	{
		for (const auto& element : m_textures)
		{
			m_textures[element.first].m_texture = prepareTextureImp(element.second.m_uri);
		}
	}

	void MaterialInst::loadTexture()
	{
		for (auto& it : m_textures)
		{
			TextureRes* texRes = it.second.m_texture;
			texRes->prepareLoad();
		}
	}

	void MaterialInst::unloadTexture()
	{
		m_textures.clear();
	}

	// 根据索引获取纹理
	TextureRes* MaterialInst::getTexture(const int& index)
	{
		auto it = m_textures.find(index);
		if (it != m_textures.end())
		{
			return it->second.m_texture;
		}

		return nullptr;
	}

	// 设置宏定义
	void MaterialInst::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());
	}

	// 判断变量是否存在
	bool MaterialInst::isUniformExist(const String& name)
	{
		return m_unifroms.find(name)!=m_unifroms.end();
	}

	// 修改变量
	void MaterialInst::setUniformValue(const String& name, const ShaderParamType& type, void* value)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
		{
			if (it->second->m_type == type)
				it->second->setValue(value);
			else
				EchoLogError("MaterialInstance::ModifyUnifromParam Type Error!");
		}
	}

	//void* MaterialInst::getUniformValue(const String& name, ShaderParamType type)
	//{
	//	if (type == SPT_TEXTURE)
	//	{
	//		void* index = getUniformValue(name);
	//		if (index)
	//		{
	//			return &m_textures[*(int*)index];
	//		}
	//		return NULL;
	//	}

	//	return getUniformValue(name);
	//}

	void MaterialInst::addTexture(int idx, const String& name)
	{
		TextureInfo info;
		info.m_idx = idx;
		info.m_name = name;
		m_textures[idx] = info;
	}

	TextureRes* MaterialInst::setTexture(const String& name, const String& uri)
	{
		return setTexture( name, prepareTextureImp(uri));
	}

	TextureRes* MaterialInst::setTexture(const String& name, TextureRes* textureRes)
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
	MaterialInst::Uniform* MaterialInst::getUniform(const String& name)
	{
		const auto& it = m_unifroms.find(name);
		if (it != m_unifroms.end())
			return it->second;

		return NULL;
	}

	// 是否使用了宏定义
	bool MaterialInst::isMacroUsed(const String& macro)
	{
		for (const String& _macro : m_macros)
		{
			if (macro == _macro)
				return true;
		}

		return false;
	}

	// 设置宏定义
	void MaterialInst::setMacro(const String& macro, bool enabled)
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
	void MaterialInst::buildRenderQueue()
	{
		// make sure macros
		String finalMacros; finalMacros.reserve(512);
		for (const String& macro : m_macros)
		{
			finalMacros += "#define " + macro + "\n";
		}

		// create material
		m_material = EchoNew(Material);
		if (m_officialMaterialContent)
			m_material->loadFromContent(m_officialMaterialContent, finalMacros);
		else if (!m_materialTemplate.empty())
			m_material->loadFromFile( m_materialTemplate, finalMacros);
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
	void MaterialInst::matchUniforms()
	{
		ShaderProgram* shaderProgram = m_material->getShaderProgram();
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
					const Material::DefaultUniform* defaultUniform = m_material->getDefaultUniformValue(uniform->m_name);
					if (defaultUniform && uniform->m_count == defaultUniform->count && uniform->m_type == defaultUniform->type)
						uniform->setValue(defaultUniform->value);

					m_unifroms[uniform->m_name] = uniform;
				}
			}
		}
	}
}