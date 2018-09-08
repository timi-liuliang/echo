#pragma once

#include <engine/core/util/AssertX.h>
#include "engine/core/render/interface/mesh/MeshVertexData.h"
#include "Shader.h"

namespace Echo
{
	// 支持的着色器参数类型
	enum ShaderParamType
	{
		SPT_UNKNOWN,
		SPT_INT,
		SPT_FLOAT,
		SPT_VEC2,
		SPT_VEC3,
		SPT_VEC4,
		SPT_MAT4,
		SPT_TEXTURE,

		SPT_MAX
	};

	/**
	* ShaderProgram 2014-11-18
	*/
	class Renderable;
	class ShaderProgramRes;
	class ShaderProgram
	{
	public:
		// 参数描述
		struct Uniform
		{
			String			m_name;			// 名称
			ShaderParamType	m_type;			// 类型
			int				m_count;		// 大小
			int				m_sizeInBytes;	// 大小
			int				m_location;		// 位置
			Byte*			m_origin_value;
			Byte*			m_value;		// 当前值
			bool			m_isDirty;		// 值是否发生了变化

			// 构造函数
			Uniform()
				: m_name("UnKnown")
				, m_type(SPT_UNKNOWN)
				, m_count(-1)
				, m_location(-1)
				, m_origin_value(NULL)
				, m_value(NULL)
				, m_isDirty(true)
			{}

			// 析构函数
			~Uniform()
			{
				ECHO_FREE(m_value);
			}

			// 重置当前值
			void resetValue()
			{
				m_isDirty = true;
			}

			// 设置值
			void setValue(const void* value)
			{
				EchoAssert(value);
				m_origin_value = (Byte*)value;

				// 分配堆空间
				if (!m_value)
				{
					m_value = (Byte*)ECHO_MALLOC(m_sizeInBytes);
				}

				// 若脏标记为true,则刚初始化，直接拷贝即可
				if (m_isDirty)
				{
					memcpy(m_value, value, m_sizeInBytes);
				}
				else if ( memcmp(m_value, value, m_sizeInBytes) != 0)
				{
					m_isDirty = true;
					memcpy(m_value, value, m_sizeInBytes);
				}
			}

		};
		typedef std::map<int, Uniform> UniformArray;

	public:
		ShaderProgram(ShaderProgramRes* material);
		virtual ~ShaderProgram();

		// 获取关连着色器
		Shader* getShader(Shader::ShaderType type) const { return m_pShaders[(ui32)type]; }

		virtual bool attachShader(Shader* pShader);
		virtual Shader* detachShader(Shader::ShaderType type);

		// 链接着色器
		virtual bool linkShaders() = 0;

		inline bool isLinked() const { return m_bLinked; }

		// 根据参数名获取参数物理地址
		virtual int	getParamPhysicsIndex(const String& paramName);

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual i32 getAtrribLocation(VertexSemantic vertexSemantic);

		// 绑定几何体数据
		virtual void bindRenderable(Renderable* renderInput) = 0;

	public:
		// 根据变量名称设置值
		void setUniform(const char* name, const void* value, ShaderParamType uniformType, ui32 count);

		// 根据物理索引设置变量值
		virtual void setUniform(ui32 physicIdx, const void* value, ShaderParamType uniformType, ui32 count);

		// 应用变量
		virtual void bindUniforms() = 0;

		// 获取变量描述
		UniformArray* getUniforms(){ return &m_uniforms; }

		// 根据变量类型获取其对应ByteSize
		static int getUniformByteSizeByUniformType(ShaderParamType uniformType);

		// 根据名称获取Uniform
		Uniform* getUniform(const String& name);

		// 检测参数合法性
		bool checkValid();

	protected:
		ShaderProgramRes*	m_material;							// 关联材质
		Shader*				m_pShaders[Shader::ST_SHADERCOUNT];
		bool				m_bLinked;
		UniformArray		m_uniforms;							// 参数信息
	};
}
