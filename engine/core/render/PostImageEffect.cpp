#include "PostImageEffect.h"
#include "PostEffectPassLensFlare.h"
#include "render/Material.h"
#include "render/ShaderProgram.h"
#include "RenderTargetManager.h"
#include "rapidxml/rapidxml.hpp"
#include "WeatherLightningImageEffect.h"
#include "BloomEffect.h"
#include "PostFrag.h"
#include "MaterialDesc.h"
#include "Fxaa.h"
#include "MaterialInst.h"

namespace Echo
{
	PostImageEffectPass::PostImageEffectPass(PostImageEffect* parent)
		: m_parent(parent)
		, m_final(false)
		, m_material(nullptr)
		, m_renderable(nullptr)
	{
	}

	PostImageEffectPass::~PostImageEffectPass()
	{
		MaterialManager::instance()->destroyMaterial(m_material);
		if (m_renderable)
		{
			RenderInput* renderinput = m_renderable->getRenderInput();
			EchoSafeDelete(renderinput, RenderInput);
			Renderer::instance()->destroyRenderables(&m_renderable, 1);
		}

		RenderTargetManager::instance()->destroyRenderTargetByID(m_renderTargetID);
	}

	Echo::String PostImageEffectPass::name() const
	{
		return m_name;
	}

	ui32 PostImageEffectPass::renderTargetID() const
	{
		return m_renderTargetID;
	}

	void PostImageEffectPass::load(rapidxml::xml_node<char>* node)
	{
		rapidxml::xml_attribute<>* name = node->first_attribute("id");
		rapidxml::xml_attribute<>* material = node->first_attribute("material");
		rapidxml::xml_attribute<>* widthAttr = node->first_attribute("width");
		rapidxml::xml_attribute<>* heightAttr = node->first_attribute("height");
		rapidxml::xml_attribute<>* scaleAttr = node->first_attribute("scale");
		rapidxml::xml_attribute<>* depthAttr = node->first_attribute("depth");
		rapidxml::xml_attribute<>* formate = node->first_attribute("formate");

		m_material = createMaterial(material->value());
		m_renderable = createRenderable(m_material);

		if (name)
		{
			RenderTargetConf conf;
			conf.name = name->value();
			conf.width = widthAttr ? StringUtil::ParseUI32(widthAttr->value()) : 0;
			conf.height = heightAttr ? StringUtil::ParseUI32(heightAttr->value()) : 0;
			conf.scale = scaleAttr ? StringUtil::ParseFloat(scaleAttr->value()) : 1.0f;
			conf.depth = depthAttr ? depthAttr->value() : StringUtil::BLANK;
			conf.formate = formate ? parseFormate(formate->value()) : PF_RGB8_UNORM;
			createRenderTarget(conf);
		}
		loadCustom(node);

		rapidxml::xml_node<>* parameter = node->first_node();
		while (parameter)
		{
			String elemName = parameter->name();
			if (elemName == "BindBuffer")
			{
				rapidxml::xml_attribute<>* name = parameter->first_attribute("sampler");
				rapidxml::xml_attribute<>* sourceRT = parameter->first_attribute("sourceRT");
				rapidxml::xml_attribute<>* slot = parameter->first_attribute("slot");
				rapidxml::xml_attribute<>* depth = parameter->first_attribute("depth");
				rapidxml::xml_attribute<>* forEffect = parameter->first_attribute("for");
				rapidxml::xml_attribute<>* mutability = parameter->first_attribute("mutable");
				Sampler sampler;
				sampler.name = name->value();
				sampler.sourceRT = sourceRT->value();
				sampler.slot = StringUtil::ParseUI32(slot->value());
				sampler.depth = !!depth;
				sampler.forEffect = forEffect ? forEffect->value() : "";
				sampler.mutability = mutability ? StringUtil::ParseBool(mutability->value()) : sampler.sourceRT.empty();
				m_samplers.push_back(sampler);
			}
			else if (elemName == "Uniform")
			{
				rapidxml::xml_attribute<>* attr = parameter->first_attribute("name");
				Uniform u;
				String uniformName = attr->value();
				u.name = uniformName;
				rapidxml::xml_attribute<>* attrFor = parameter->first_attribute("for");
				if (attrFor)
				{
					u.forEffect = attrFor->value();
					attr = attrFor;
				}
				std::vector<float> uniformValue;
				for (int i = 0; attr->next_attribute(); ++i)
				{
					rapidxml::xml_attribute<>* value = attr->next_attribute();
					uniformValue.push_back(StringUtil::ParseFloat(value->value()));
					attr = value;
				}
				u.values = uniformValue;
				m_uniforms.push_back(u);
			}
			else if (elemName == "CustomUniform")
			{
				rapidxml::xml_attribute<>* attr = parameter->first_attribute("name");
				String uniformName = attr->value();
				Vector4 uniformValue;
				for (int i = 0; i < 4 && attr->next_attribute(); ++i)
				{
					rapidxml::xml_attribute<>* value = attr->next_attribute();
					uniformValue[i] = StringUtil::ParseFloat(value->value());
					attr = value;
				}

				setCustomParameter(uniformName, uniformValue);
			}
			else
			{
				EchoLogError("Unknown element named: %s", elemName.c_str());
			}
			parameter = parameter->next_sibling();
		}
		initializeShaderParams();
	}

	void PostImageEffectPass::process()
	{
		auto rendtargetMgr = RenderTargetManager::instance();
		auto renderTarget = m_renderTargetID;
		rendtargetMgr->beginRenderTarget(renderTarget, false, Renderer::BGCOLOR, false);
		for (const auto& sampler : m_samplers)
		{
			if (sampler.inUsed)
			{
				ui32 sourceRT = m_parent->getSourceRT();

				if (sampler.depth)
				{
					RenderTarget* rt = rendtargetMgr->getRenderTargetByID(sourceRT);
					Texture* tex = rt->getDepthTexture();
					m_renderable->setTexture(sampler.slot, tex, m_material->getSamplerState(sampler.slot));
				}
				else
				{
					if (!sampler.mutability)
					{
						sourceRT = rendtargetMgr->getRenderTargetID(sampler.sourceRT);
					}
					RenderTarget* rt = rendtargetMgr->getRenderTargetByID(sourceRT);
					Texture* tex = rt->getBindTexture();
					m_renderable->setTexture(sampler.slot, tex, m_material->getSamplerState(sampler.slot));
				}
			}
		}

		RenderTarget* rt = rendtargetMgr->getRenderTargetByID(renderTarget);
		auto it = std::find_if(
			std::begin(m_uniforms),
			std::end(m_uniforms), [](const Uniform& u) { return u.name == "RTSize"; });
		if (it != m_uniforms.end())
		{
			std::vector<float>& vec = it->values;
			vec.resize(2);
			vec[0] = static_cast<Real>(rt->width());
			vec[1] = static_cast<Real>(rt->height());
		}

		m_renderable->render();
		rendtargetMgr->endRenderTarget(renderTarget);
		bool invalidateDepth = !m_final;
		rendtargetMgr->invalidateFrameBuffer(renderTarget, true, invalidateDepth, false);
	}

	void PostImageEffectPass::setParameter(const String& uniform, const Vector4& value)
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
		else
		{
			setCustomParameter(uniform, value);
		}
	}

	bool PostImageEffectPass::find(const String& uniform, Vector4& value) const
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

	bool PostImageEffectPass::createRenderTarget(const RenderTargetConf& conf)
	{
		m_name.assign(conf.name);
		bool fixed = conf.width && conf.height;
		ui32 screenWidth = conf.width ? conf.width : Renderer::instance()->getScreenWidth();
		ui32 screenHeight = conf.height ? conf.height : Renderer::instance()->getScreenHeight();
		ui32 w = static_cast<ui32>(screenWidth * conf.scale);
		ui32 h = static_cast<ui32>(screenHeight * conf.scale);

		auto renderTargetManager = RenderTargetManager::instance();
		ui32 rtExist = renderTargetManager->getRenderTargetID(conf.name);

		m_renderTargetID = (rtExist == RTI_End) ? renderTargetManager->allocateRenderTargetID(conf.name) : rtExist;

		RenderTarget::Options option;
		if (conf.depth != StringUtil::BLANK)
		{
			ui32 id = renderTargetManager->getRenderTargetID(conf.depth);
			option.depthTarget = renderTargetManager->getRenderTargetByID(id);
		}

		auto rt = (rtExist == RTI_End) ? renderTargetManager->createRenderTarget(m_renderTargetID, w, h, conf.formate, option) : renderTargetManager->getRenderTargetByID(rtExist);
		if (rt)
		{
			rt->setScaleFactor(conf.scale);
			rt->setResolutionRelative(!fixed);
		}
		else
		{
			EchoLogError("RenderTargetManager::createRenderTarget( %s ) ... Failed", conf.name.c_str());
			return false;
		}
		return true;
	}

	Material* PostImageEffectPass::createMaterial(const String& fileName)
	{
		Material* retMaterial = MaterialManager::instance()->createMaterial();
		retMaterial->loadFromFile(fileName, "");
		return retMaterial;
	}

	Renderable* PostImageEffectPass::createRenderable(Material* material)
	{
		ShaderProgram* shader = material->getShaderProgram();
		Renderable* retRenderable = Renderer::instance()->createRenderable(NULL, material);

		RenderInput* renderInput = Renderer::instance()->createRenderInput(shader);
		renderInput->bindVertexStream(*m_parent->m_verticesList, m_parent->m_verticesBuffer);
		renderInput->bindIndexStream(m_parent->m_indicesBuffer);
		retRenderable->setRenderInput(renderInput);

		return retRenderable;
	}

	void PostImageEffectPass::loadCustom(rapidxml::xml_node<char>* node)
	{
		//(void*)node;
	}

	void PostImageEffectPass::initializeShaderParams()
	{
		// global uniform
		ShaderProgram* shader = m_material->getShaderProgram();
		auto renderTargetSize = shader->getUniform("RTSize");
		if (renderTargetSize)
		{
			m_uniforms.push_back(Uniform("RTSize", std::vector<float>(4, 0)));
		}

		size_t count = m_samplers.size() + m_uniforms.size();
		m_renderable->beginShaderParams(count);
		for (const auto& sampler : m_samplers)
		{
			int loc = shader->getParamPhysicsIndex(sampler.name);
			m_renderable->setShaderParam(loc, SPT_TEXTURE, (void*)&sampler.slot);
		}
		for (const auto& uniform : m_uniforms)
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
		m_renderable->endShaderParams();
	}

	void PostImageEffectPass::setCustomParameter(const String & uniform, const Vector4 & value)
	{
		EchoLogInfo("uniform [%s] is not in this pass", uniform.c_str());
	}

	bool PostImageEffectPass::getCustomParameter(const String& name, Vector4& value) const
	{
		EchoLogInfo("parameter [%s] is not in this pass", name.c_str());
		return false;
	}

	PixelFormat PostImageEffectPass::parseFormate(const String& value)
	{
		for (i32 i = 0; i < PF_COUNT; ++i)
		{
			if (value == s_PixelFormat[i])
			{
				return static_cast<PixelFormat>(i);
			}
		}

		return PF_UNKNOWN;
	}

	//////////////////////////////////////////////////////////////////////////

	PostImageEffect::PostImageEffect(const String& name) :
		m_name(name),
		m_sourceRTID(RTI_LDRSceneColorMap),
		m_enable(false),
		m_final(false)
	{
	}

	PostImageEffect::~PostImageEffect()
	{
		EchoSafeDeleteContainer(m_passList, PostImageEffectPass);
	}

	void PostImageEffect::setFinal(bool value)
	{
		m_final = value;
		m_passList.back()->setFinal(value);
	}

	void PostImageEffect::setParameter(const String& name, const Vector4& value)
	{
		for (auto& pass : m_passList)
		{
			pass->setParameter(name, value);
		}
	}

	void PostImageEffect::setLogicCalcParameter(const String& namej, const String& value) {}

	String PostImageEffect::getLogicCalcParameter(const String& name) { return ""; };

	Vector4 PostImageEffect::getParameter(const String& name) const
	{
		Vector4 value;
		for (auto& pass : m_passList)
		{
			if (pass->find(name, value))
			{
				return value;
			}
		}
		EchoAssertX("PostImageEffect[%s] is not contain uniform[%s]", m_name.c_str(), name.c_str());
		return value;
	}

	void PostImageEffect::loadFromFile(const String& filename)
	{
		if (filename.empty())
		{
			return;
		}
		MemoryReader memReader(filename.c_str());
		rapidxml::xml_document<> doc;
		doc.parse<0>(memReader.getData<char*>());
		rapidxml::xml_node<>* pipeline = doc.first_node();

		if (!pipeline)
		{
			EchoLogError("invalid PostImageEffect pipeline define file.");
			return;
		}
		rapidxml::xml_node<>* stage = pipeline->first_node();
		while (stage)
		{
			rapidxml::xml_attribute<>* custom = stage->first_attribute("custom");
			String name = custom ? custom->value() : "";
			PostImageEffectPass* pass = PostImageEffectFactory::createPass(name, this);
			pass->load(stage);
			m_passList.push_back(pass);
			stage = stage->next_sibling();
		}
	}

	void PostImageEffect::setGPUBuffers(RenderInput::VertexElementList* verticesList, GPUBuffer* verticesBuffer, GPUBuffer* indicesBuffer)
	{
		m_verticesList = verticesList;
		m_verticesBuffer = verticesBuffer;
		m_indicesBuffer = indicesBuffer;
	}

	ui32 PostImageEffect::outputRenderTarget() const
	{
		return m_final ? m_passList.back()->renderTargetID() : m_sourceRTID;
	}

	void PostImageEffect::render()
	{
		if (m_enable)
		{
			for (const auto& pass : m_passList)
			{
				pass->process();
			}
		}
	}

	void PostImageEffect::setForEffect(const String & name, const String & uniform, const Vector4 & value)
	{
		ECHO_DOWN_CAST<PostFrag*>(m_passList.back())->setForEffect(name, uniform, value);
	}

	bool PostImageEffect::findForEffect(const String & name, const String & uniform, Vector4 & value) const
	{
		return ECHO_DOWN_CAST<PostFrag*>(m_passList.back())->findForEffect(name, uniform, value);;
	}

	//////////////////////////////////////////////////////////////////////////

	PostImageEffect* PostImageEffectFactory::create(const String& name)
	{
		if (name == "WeatherThunder")
		{
			return EchoNew(WeatherLightningImageEffect)(name);
		}
		else
		{
			return EchoNew(PostImageEffect)(name);
		}
	}

	PostImageEffectPass * PostImageEffectFactory::createPass(const String & name, PostImageEffect* parent)
	{
		if (name == "PostEffectPassLensFlare")
		{
			return EchoNew(PostEffectPassLensFlare)(parent);
		}
		else if (name == "BloomEffectPass0")
		{
			return EchoNew(BloomEffectPass0)(parent);
		}
		else if (name == "BloomEffectPass1")
		{
			return EchoNew(BloomEffectPass1)(parent);
		}
		else if (name == "PostFrag")
		{
			return EchoNew(PostFrag)(parent);
		}
		else if (name == "Fxaa")
		{
			return EchoNew(Fxaa)(parent);
		}
		else
		{
			return EchoNew(PostImageEffectPass)(parent);
		}
	}

}  // namespace Echo
