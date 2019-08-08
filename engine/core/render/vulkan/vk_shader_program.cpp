#include "vk_shader_program.h"
#include "vk_renderer.h"
#include "engine/core/render/interface/glslcc/GLSLCrossCompiler.h"

namespace Echo
{
	static bool createShader(const vector<ui32>::type& spirv, VkShaderModule& vkShader)
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		VkShaderModuleCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.codeSize = spirv.size() * sizeof(ui32);
		createInfo.pCode = spirv.data();

		if (VK_SUCCESS != vkCreateShaderModule(vkRenderer->getVkDevice(), &createInfo, nullptr, &vkShader))
		{
			EchoLogError("Vulkan create shader failed");

			return false;
		}

		return true;
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
}