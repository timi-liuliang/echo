#include "PostFrag.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/Renderer.h"
#include "RenderStageManager.h"
#include <bitset>
#include "engine/core/render/MaterialInstance.h"

namespace Echo
{

	PostFrag::PostFrag(PostImageEffect* parent)
		: PostImageEffectPass(parent)
		, m_index((std::numeric_limits<ulong>::max)())
	{
		auto manager = RenderStageManager::instance();
		m_renderStage = (PostProcessRenderStage*)manager->getRenderStageByID(Echo::RSI_PostProcess);
	}

	PostFrag::~PostFrag()
	{
		m_materials.erase(
			std::remove_if(
				std::begin(m_materials),
				std::end(m_materials),
				[this](const std::pair<Material*, Renderable*>& p) {
			return p.first == m_material && p.second == m_renderable;
		}));
		for (auto& m : m_materials)
		{
			MaterialManager::instance()->destroyMaterial(m.first);
			RenderInput* renderinput = m.second->getRenderInput();
			EchoSafeDelete(renderinput, RenderInput);
			Renderer::instance()->destroyRenderables(&m.second, 1);
		}
	}

	void PostFrag::process()
	{
		initializeShaderParams();
		PostImageEffectPass::process();
	}

	void PostFrag::setParameter(const String & uniform, const Vector4 & value)
	{
		auto it = std::find_if(
			std::begin(m_uniforms),
			std::end(m_uniforms), [uniform](const Uniform& u) { return u.name == uniform; });
		if (it != m_uniforms.end())
		{
			std::vector<float>& vec = it->values;
			for (size_t i = 0; i < vec.size(); ++i)
				vec[i] = value[i];
		}
	}

	bool PostFrag::find(const String& uniform, Vector4& value) const
	{
		auto it = std::find_if(
			std::begin(m_uniforms),
			std::end(m_uniforms), [uniform](const Uniform& u) { return u.name == uniform; });
		if (it != m_uniforms.end())
		{
			const std::vector<float>& vec = it->values;
			for (size_t i = 0; i < vec.size(); ++i)
				value[i] = vec[i];
			return true;;
		}
		return getCustomParameter(uniform, value);
	}

	void PostFrag::setForEffect(const String& name, const String& uniform, const Vector4& value)
	{
		auto it = std::find_if(
			std::begin(m_uniforms),
			std::end(m_uniforms), [name, uniform](const Uniform& u) {
			return u.forEffect == name && u.name == uniform;
		});
		if (it != m_uniforms.end())
		{
			std::vector<float>& vec = it->values;
			for (size_t i = 0; i < vec.size() && i < 4; ++i)
			{
				vec[i] = value[i];
			}
		}
	}

	bool PostFrag::findForEffect(const String& name, const String& uniform, Vector4& value) const
	{
		auto it = std::find_if(
			std::begin(m_uniforms),
			std::end(m_uniforms), [name, uniform](const Uniform& u) {
			return u.forEffect == name && u.name == uniform;
		});
		if (it != m_uniforms.end())
		{
			const std::vector<float>& vec = it->values;
			for (size_t i = 0; i < vec.size() && i < 4; ++i)
			{
				value[i] = vec[i];
			}
			return true;
		}
		return false;
	}

	Material * PostFrag::createMaterial(const String & fileName)
	{
		m_effects = m_renderStage->getAllEffect();
		int materialCount = (int)std::pow(2, m_effects.size());
		m_materials.reserve(materialCount);
		for (int i = 0; i < materialCount; ++i)
		{
			std::bitset<8> b(i);
			String macro;
			for (size_t j = 0; j < m_effects.size(); ++j)
			{
				if (b[j])
				{
					String e(m_effects[j]);
					StringUtil::UpperCase(e);
					macro += "#define " + e + "\n";
				}
			}
			auto m = MaterialManager::instance()->createMaterial();
			m->loadFromFile(fileName, macro);
			m_materials.push_back(std::make_pair(m, PostImageEffectPass::createRenderable(m)));
		}
		return m_materials.back().first;
	}

	Renderable * PostFrag::createRenderable(Material * material)
	{
		return m_materials.back().second;
	}

	void PostFrag::initializeShaderParams()
	{
		std::bitset<8> b;
		size_t shaderParamCount = m_samplers.size() + m_uniforms.size();
		for (size_t i = 0; i < m_effects.size(); ++i)
		{
			b[i] = m_renderStage->getImageEffectEnable(m_effects[i]);
			for (auto& iter : m_samplers)
			{
				if (iter.forEffect == m_effects[i])
				{
					iter.inUsed = b[i];
					if (!b[i])
					{
						--shaderParamCount;
					}
				}
			}

			for (auto& iter : m_uniforms)
			{
				if (iter.forEffect == m_effects[i])
				{
					iter.inUsed = b[i];
					if (!b[i])
					{
						--shaderParamCount;
					}
				}
			}
		}
		auto index = b.to_ulong();
		if (m_index == index) return;

		m_index = index;
		m_material = m_materials[m_index].first;
		m_renderable = m_materials[m_index].second;

		// global uniform
		ShaderProgram* shader = m_material->getShaderProgram();
		m_renderable->beginShaderParams(shaderParamCount);
		for (const auto& sampler : m_samplers)
		{
			if (sampler.inUsed)
			{
				int loc = shader->getParamPhysicsIndex(sampler.name);
				m_renderable->setShaderParam(loc, SPT_TEXTURE, (void*)&sampler.slot);
			}
		}
		for (const auto& uniform : m_uniforms)
		{
			if (uniform.inUsed)
			{
				const std::vector<float>& vec = uniform.values;
				if (vec.size() == 0)
					continue;

				auto uniformPtr = shader->getUniform(uniform.name);
				if (!uniformPtr)
				{
					EchoLogError("[PostImageEffectPass:%d]::Uniform Param %s is Not Exist.", __LINE__, uniform.name.c_str());
					continue;
				}

				auto type = shader->getUniform(uniform.name)->m_type;
				int loc = shader->getParamPhysicsIndex(uniform.name);

				const void*     data = static_cast<const void*>(&vec[0]);
				const size_t    datalen = vec.size();
				m_renderable->setShaderParam(loc, type, data, datalen);
			}
		}
		m_renderable->endShaderParams();
	}
}  // namespace Echo
