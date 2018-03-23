#pragma once

#include "Core.h"
#include "render/RenderInput.h"

namespace rapidxml
{
	template<typename Ch> class xml_node;
}

namespace Echo
{
	class Material;
	class PostImageEffect;
	class PostImageEffectPass
	{
	public:
		struct Sampler
		{
			Sampler()
				: slot((std::numeric_limits<ui32>::max)())
				, depth(false)
				, mutability(false)
				, inUsed(true)
			{
			}

			String name;
			String sourceRT;
			ui32 slot;
			String forEffect;
			bool depth;
			bool mutability;
			bool inUsed;
		};
		struct Uniform
		{
			Uniform()
			{
			}

			Uniform(const String& n, const std::vector<float>& v)
				: name(n)
				, values(v)
				, inUsed(true)
			{
			}

			String name;
			String forEffect;
			std::vector<float> values;
			bool inUsed;
		};
		struct RenderTargetConf
		{
			String name;
			ui32 width;
			ui32 height;
			float scale;
			String depth;
			PixelFormat formate;

			RenderTargetConf()
				: width(0)
				, height(0)
				, scale(1.0)
				, depth(StringUtil::BLANK)
				, formate(PF_RGBA8_UNORM)
			{}
		};
	public:
		PostImageEffectPass(PostImageEffect* parent);
		virtual ~PostImageEffectPass();
		String name() const;
		ui32 renderTargetID() const;
		void setFinal(bool value) { m_final = value; }
		virtual void load(rapidxml::xml_node<char>* node);
		virtual void process();
		virtual void setParameter(const String& uniform, const Vector4& value);
		virtual bool find(const String& uniform, Vector4& value) const;
		virtual void setCustomParameter(const String& uniform, const Vector4& value);
		virtual bool getCustomParameter(const String& name, Vector4& value) const;

		PixelFormat parseFormate(const String& value);

	protected:
		virtual bool createRenderTarget(const RenderTargetConf& conf);
		virtual Material* createMaterial(const String& fileName);
		virtual Renderable* createRenderable(Material* material);

		virtual void loadCustom(rapidxml::xml_node<char>* node);
		virtual void initializeShaderParams();

		String m_name;
		bool m_final;
		Material* m_material;
		Renderable* m_renderable;
		PostImageEffect* m_parent;
		ui32 m_renderTargetID;
		vector<Sampler>::type m_samplers;
		vector<Uniform>::type m_uniforms;
	};

	class PostImageEffect
	{
		friend class PostImageEffectPass;
	public:
		PostImageEffect(const String& name);
		virtual~PostImageEffect();

		String name() const { return m_name; }
		virtual void setEnable(bool enable) { m_enable = enable; }
		virtual void setFinal(bool value);
		virtual bool getEnable() const { return m_enable; }
		virtual void setParameter(const String& name, const Vector4& value);
		virtual void setLogicCalcParameter(const String& name, const String& value);
		virtual String getLogicCalcParameter(const String& name);
		virtual Vector4 getParameter(const String& name) const;
		virtual void loadFromFile(const String& filename);
		virtual void setGPUBuffers(RenderInput::VertexElementList* verticesList, GPUBuffer* verticesBuffer, GPUBuffer* indicesBuffer);
		virtual ui32 outputRenderTarget() const;
		virtual void render();
		virtual void setSourceRT(const ui32 id) { m_sourceRTID = id; }
		virtual ui32 getSourceRT() const { return m_sourceRTID; }

		void setForEffect(const String& name, const String& uniform, const Vector4& value);
		bool findForEffect(const String& name, const String& uniform, Vector4& value) const;

	protected:
		String m_name;
		RenderInput::VertexElementList* m_verticesList;
		GPUBuffer* m_verticesBuffer;
		GPUBuffer* m_indicesBuffer;
		vector<PostImageEffectPass*>::type m_passList;
		ui32 m_sourceRTID;
		bool m_enable;
		bool m_final;
	};

	class PostImageEffectFactory
	{
	public:
		static PostImageEffect* create(const String& name);
		static PostImageEffectPass* createPass(const String& name, PostImageEffect* parent);
	};
}  // namespace Echo
