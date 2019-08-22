#include "vk_shader_program.h"
#include "vk_renderer.h"
#include "engine/core/render/interface/glslcc/GLSLCrossCompiler.h"

namespace Echo
{
	static bool createShader(const vector<ui32>::type& spirv, VkShaderModule& vkShader)
	{
        if (!spirv.empty())
        {
            VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

            VkShaderModuleCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.codeSize = spirv.size() * sizeof(ui32);
            createInfo.pCode = spirv.data();

            if (VK_SUCCESS == vkCreateShaderModule(vkRenderer->getVkDevice(), &createInfo, nullptr, &vkShader))
                return true;
        }

        EchoLogError("Vulkan create shader failed");
        return false;
	}

	VKShaderProgram::~VKShaderProgram()
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		vkDestroyShaderModule(vkRenderer->getVkDevice(), m_vkVertexShader, nullptr);
		vkDestroyShaderModule(vkRenderer->getVkDevice(), m_vkFragmentShader, nullptr);
	}

	bool VKShaderProgram::createShaderProgram(const String& vsSrc, const String& psSrc)
	{
		GLSLCrossCompiler glslCompiler;
		glslCompiler.setInput(vsSrc.c_str(), psSrc.c_str(), nullptr);

		bool isCreateVSSucceed = createShader(glslCompiler.getSPIRV(GLSLCrossCompiler::ShaderType::VS), m_vkVertexShader);
		bool isCreateFSSucceed = createShader(glslCompiler.getSPIRV(GLSLCrossCompiler::ShaderType::FS), m_vkFragmentShader);
		m_isValid = isCreateVSSucceed && isCreateFSSucceed;

		// create shader stage
		if (m_isValid)
		{
			VkPipelineShaderStageCreateInfo shaderStagesInfo[2] = { {}, {} };

			shaderStagesInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStagesInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStagesInfo[0].module = m_vkVertexShader;
			shaderStagesInfo[0].pName = "main";

			shaderStagesInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStagesInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStagesInfo[1].module = m_vkFragmentShader;
			shaderStagesInfo[1].pName = "main";
		}

		return m_isValid;
	}

    void VKShaderProgram::createVkUniformBuffer()
    {
        // this is not good here, reuse VkBuffer afterwards
        EchoSafeDelete(m_vkVertexShaderUniformBuffer, VKBuffer);
        EchoSafeDelete(m_vkFragmentShaderUniformBuffer, VKBuffer);

        Buffer vertUniformBuffer(m_vertexShaderUniformBytes.size(), m_vertexShaderUniformBytes.data(), false);
        m_vkVertexShaderUniformBuffer = EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_UNIFORM, GPUBuffer::GBU_DYNAMIC, vertUniformBuffer));

        Buffer fragmentUniformBuffer(m_fragmentShaderUniformBytes.size(), m_fragmentShaderUniformBytes.data(), false);
        m_vkFragmentShaderUniformBuffer = EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_UNIFORM, GPUBuffer::GBU_DYNAMIC, fragmentUniformBuffer));
    }

    void VKShaderProgram::createVkDescriptorAndPipelineLayouts()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        VkDescriptorSetLayoutBinding layoutBindings[2];
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        layoutBindings[0].pImmutableSamplers = nullptr;

        if (false /*use texture*/)
        {
            layoutBindings[1].binding = 1;
            layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindings[1].descriptorCount = 1;
            layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindings[1].pImmutableSamplers = nullptr;
        }

        // create a descriptor set layout based on layout bindings
        VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
        dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslCreateInfo.pNext = nullptr;
        dslCreateInfo.flags = 0;
        dslCreateInfo.bindingCount = 1;
        dslCreateInfo.pBindings = layoutBindings;

        if (VK_SUCCESS != vkCreateDescriptorSetLayout(vkRenderer->getVkDevice(), &dslCreateInfo, nullptr, &m_vkDescriptorSetLayout))
        {
            EchoLogError("create descriptor set layout.");
            return;
        }

        VkPipelineLayoutCreateInfo plCreateInfo = {};
        plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plCreateInfo.pNext = nullptr;
        plCreateInfo.pushConstantRangeCount = 0;
        plCreateInfo.pPushConstantRanges = nullptr;
        plCreateInfo.setLayoutCount = 1;
        plCreateInfo.pSetLayouts = &m_vkDescriptorSetLayout;

        if (VK_SUCCESS != vkCreatePipelineLayout(vkRenderer->getVkDevice(), &plCreateInfo, nullptr, &m_vkPipelineLayout))
        {
            EchoLogError("vulkan create pipeline layout failed");
        }
    }

    void VKShaderProgram::parseUniforms()
    {
        allocUniformBytes();
    }

    // alloc uniform bytes
    void VKShaderProgram::allocUniformBytes()
    {
        m_vertexShaderUniformBytes.clear();
        m_fragmentShaderUniformBytes.clear();
    }

    void VKShaderProgram::bindUniforms()
    {
        // organize uniform bytes

        // update uniform VkBuffer by memory
        createVkUniformBuffer();

        // set uniforms
        createVkDescriptorAndPipelineLayouts();
    }

    void VKShaderProgram::bindRenderable(Renderable* renderable)
    {

    }
}