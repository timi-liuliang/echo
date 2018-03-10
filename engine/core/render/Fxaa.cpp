#include "fxaa.h"
#include "render/Material.h"
#include "RenderStageManager.h"
#include "RenderTargetManager.h"

namespace Echo
{

	Fxaa::Fxaa(PostImageEffect* parent)
		: PostImageEffectPass(parent)
	{
	}

	Fxaa::~Fxaa()
	{
	}

	void Fxaa::setCustomParameter(const String& uniform, const Vector4& value)
	{
		if (uniform == "uResolution")
		{
			Uniform u;
			u.name = uniform;
			u.values.push_back(static_cast<float>(Renderer::instance()->getScreenWidth()));
			u.values.push_back(static_cast<float>(Renderer::instance()->getScreenHeight()));

			m_uniforms.push_back(u);
		}
	}

	bool Fxaa::getCustomParameter(const String& name, Vector4& value) const
	{
		if (name == "uResolution")
		{
			value.x = static_cast<float>(Renderer::instance()->getScreenWidth());
			value.y = static_cast<float>(Renderer::instance()->getScreenHeight());

			return true;
		}

		return false;
	}

	void Fxaa::load(rapidxml::xml_node<char>* node)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		PostImageEffectPass::load(node);
#endif
	}

	void Fxaa::process()
	{
		RenderStage* renderStage = RenderStageManager::instance()->getRenderStageByID(RSI_PostProcess);
		PostProcessRenderStage* postprocessStage = static_cast<PostProcessRenderStage*>(renderStage);
		postprocessStage->setImageEffectSourceRT(RTI_LDRSceneColorMap);

		size_t size = m_uniforms.size();
		for (size_t i = 0; i < size; ++i)
		{
			Uniform& u = m_uniforms[i];
			if (u.name == "uResolution")
			{
				u.values[0] = static_cast<float>(Renderer::instance()->getScreenWidth());
				u.values[1] = static_cast<float>(Renderer::instance()->getScreenHeight());
			}
		}

		PostImageEffectPass::process();
		
		postprocessStage->setImageEffectSourceRT(m_renderTargetID);
	}

}  // namespace Echo
