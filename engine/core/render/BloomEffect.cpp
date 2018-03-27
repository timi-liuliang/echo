#include "BloomEffect.h"
#include "render/Material.h"
#include "render/Renderer.h"
#include "MaterialInst.h"

namespace Echo
{
	// 采样次数和BloomSize之间有对应关系
	// 最初默认采样次数为16 BloomSize为0.02
	// 但是由于模糊次数太多导致效率有问题所以改成6次 所以对应的BloomSize大概是0.007
	// 修改了一下可以从pipeline传递Customer的参数
	// 修改Shader的采样次数通过宏来控制
	// 有空可以做个动态调整
	#define BLOOM_SAMPLER_COUNT 32

	BloomEffectPass0::BloomEffectPass0(PostImageEffect * parent)
		: PostImageEffectPass(parent)
		, m_parent(parent)
		, m_kernel({})
		, m_bloomSize(0.02f)
	{

	}

	BloomEffectPass0::~BloomEffectPass0()
	{

	}

	void BloomEffectPass0::initializeShaderParams()
	{
		ShaderProgram* shader = m_material->getShaderProgram();
		ui32 count = m_samplers.size() + 1;
		m_renderable->beginShaderParams(count);
		for (const auto& sampler : m_samplers)
		{
			int loc = shader->getParamPhysicsIndex(sampler.name);
			m_renderable->setShaderParam(loc, SPT_TEXTURE, (void*)&sampler.slot);
		}
		int loc = shader->getParamPhysicsIndex("uKernel");
		m_renderable->setShaderParam(loc, SPT_VEC4, (void*)&m_kernel, BLOOM_SAMPLER_COUNT);
		m_renderable->endShaderParams();
	}

	void BloomEffectPass0::setCustomParameter(const String & name, const Vector4 & value)
	{
		if (name == "bloomSize")
		{
			m_bloomSize = value.x;
		}
		float c = 0.f;
		for (auto d = 0; d < BLOOM_SAMPLER_COUNT; ++d)
		{
			float e = -1 + 2 * d / (BLOOM_SAMPLER_COUNT - 1.0f);
			float f = 4 * e;
			f = expf(-0.5f * f * f / 1.f) / 2.50662827463f;
			c += f;
			m_kernel[4 * d + 0] = e * m_bloomSize;
			m_kernel[4 * d + 1] = 0;
			m_kernel[4 * d + 2] = f;
			m_kernel[4 * d + 3] = 0;
		}
		for (auto d = 0; d < BLOOM_SAMPLER_COUNT; ++d) m_kernel[4 * d + 2] /= c;
	}

	bool BloomEffectPass0::getCustomParameter(const String& name, Vector4& value) const
	{
		if (name == "bloomSize")
		{
			value = Vector4(m_bloomSize, 0, 0, 0);
			return true;
		}
		else
		{
			return false;
		}
	}

	Material* BloomEffectPass0::createMaterial(const String& fileName)
	{
		Material* retMaterial = EchoNew(Material);
		String marcos = StringUtil::Format("#define SAMPLER %d\n", BLOOM_SAMPLER_COUNT);

		retMaterial->loadFromFile(fileName, marcos);

		return retMaterial;
	}

	//////////////////////////////////////////////////////////////////////////

	BloomEffectPass1::BloomEffectPass1(PostImageEffect * parent)
		: PostImageEffectPass(parent)
		, m_parent(parent)
		, m_kernel({})
		, m_bloomSize(0.02f)
	{

	}

	BloomEffectPass1::~BloomEffectPass1()
	{

	}

	void BloomEffectPass1::setCustomParameter(const String & name, const Vector4 & value)
	{
		if (name == "bloomSize")
		{
			m_bloomSize = value.x;
		}
		float c = 0.f;
		float ratio = (float)Renderer::instance()->getScreenWidth() / Renderer::instance()->getScreenHeight();
		for (auto d = 0; d < BLOOM_SAMPLER_COUNT; ++d)
		{
			float e = -1 + 2 * d / (BLOOM_SAMPLER_COUNT - 1.0f);
			float f = 4 * e;
			f = expf(-0.5f * f * f / 1.f) / 2.50662827463f;
			c += f;
			m_kernel[4 * d + 0] = 0;
			m_kernel[4 * d + 1] = e * m_bloomSize * ratio;
			m_kernel[4 * d + 2] = f;
			m_kernel[4 * d + 3] = 0;
		}
		for (auto d = 0; d < BLOOM_SAMPLER_COUNT; ++d)
		{
			m_kernel[4 * d + 2] /= c;
		}
	}

	bool BloomEffectPass1::getCustomParameter(const String & name, Vector4 & value) const
	{
		if (name == "bloomSize")
		{
			value = Vector4(m_bloomSize, 0, 0, 0);
			return true;
		}
		else
		{
			return false;
		}
	}

	Material* BloomEffectPass1::createMaterial(const String& fileName)
	{
		Material* retMaterial = EchoNew(Material);
		String marcos = StringUtil::Format("#define SAMPLER %d\n", BLOOM_SAMPLER_COUNT);

		retMaterial->loadFromFile(fileName, marcos);

		return retMaterial;
	}

	void BloomEffectPass1::initializeShaderParams()
	{
		ShaderProgram* shader = m_material->getShaderProgram();
		ui32 count = m_samplers.size() + 1;
		m_renderable->beginShaderParams(count);
		for (const auto& sampler : m_samplers)
		{
			int loc = shader->getParamPhysicsIndex(sampler.name);
			m_renderable->setShaderParam(loc, SPT_TEXTURE, (void*)&sampler.slot);
		}
		int loc = shader->getParamPhysicsIndex("uKernel");
		m_renderable->setShaderParam(loc, SPT_VEC4, (void*)&m_kernel, BLOOM_SAMPLER_COUNT);
		m_renderable->endShaderParams();
	}

}  // namespace Echo
