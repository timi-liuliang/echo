#include "vk_shader_program.h"
#include "vk_renderer.h"
#include "vk_mapping.h"
#include "engine/core/render/interface/glslcc/GLSLCrossCompiler.h"

namespace Echo
{
	static bool createShader(const vector<ui32>::type& spirv, VkShaderModule& vkShader, spirv_cross::Compiler*& shaderCompiler)
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

            // reflect
            shaderCompiler = EchoNew(spirv_cross::Compiler(spirv));

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

		bool isCreateVSSucceed = createShader(glslCompiler.getSPIRV(GLSLCrossCompiler::ShaderType::VS), m_vkVertexShader, m_vertexShaderCompiler);
		bool isCreateFSSucceed = createShader(glslCompiler.getSPIRV(GLSLCrossCompiler::ShaderType::FS), m_vkFragmentShader, m_fragmentShaderCompiler);
		m_isLinked = isCreateVSSucceed && isCreateFSSucceed;

		// create shader stage
		if (m_isLinked)
		{
            m_vkShaderStagesCreateInfo.assign({});
            m_vkShaderStagesCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_vkShaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            m_vkShaderStagesCreateInfo[0].module = m_vkVertexShader;
            m_vkShaderStagesCreateInfo[0].pName = "main";

            m_vkShaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_vkShaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            m_vkShaderStagesCreateInfo[1].module = m_vkFragmentShader;
            m_vkShaderStagesCreateInfo[1].pName = "main";

            parseUniforms();
		}

		return m_isLinked;
	}

    void VKShaderProgram::createVkUniformBuffer()
    {
        // organize uniform bytes
        for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            Uniform& uniform = it->second;
            if (uniform.m_value && uniform.m_type != SPT_UNKNOWN)
            {
                vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
                if (uniform.m_type != SPT_TEXTURE)
                {
                    std::memcpy(uniformBytes.data() + uniform.m_location, uniform.m_value, uniform.m_sizeInBytes * sizeof(Byte));
                }
                else
                {

                }
            }
        }

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
        m_uniforms.clear();

        // vertex uniforms
        for (auto& resource : m_vertexShaderCompiler->get_shader_resources().uniform_buffers)
            addUniform(resource, ShaderType::VS);

        // fragment uniforms
        for (auto& resource : m_fragmentShaderCompiler->get_shader_resources().uniform_buffers)
            addUniform(resource, ShaderType::FS);

        allocUniformBytes();
    }

    void VKShaderProgram::addUniform(spirv_cross::Resource& resource, ShaderType shaderType)
    {
        spirv_cross::Compiler* compiler = shaderType == ShaderType::VS ? m_vertexShaderCompiler : m_fragmentShaderCompiler;
        const spirv_cross::SPIRType& type = compiler->get_type(resource.base_type_id);
        size_t memberCount = type.member_types.size();
        for (size_t i = 0; i < memberCount; i++)
        {
            Uniform desc;
            desc.m_name = compiler->get_member_name(type.self, i);
            desc.m_shader = shaderType;
            desc.m_sizeInBytes = compiler->get_declared_struct_member_size(type, i);
            desc.m_type = VKMapping::MapUniformType(compiler->get_type(type.member_types[i]));
            desc.m_count = desc.m_sizeInBytes / MapUniformTypeSize(desc.m_type);
            desc.m_location = compiler->type_struct_member_offset(type, i);
            m_uniforms[desc.m_name] = desc;
        }
    }

    void VKShaderProgram::allocUniformBytes()
    {
        m_vertexShaderUniformBytes.clear();
        m_fragmentShaderUniformBytes.clear();

        for (auto& it : m_uniforms)
        {
            const Uniform& uniform = it.second;
            vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
            i32 bytes = uniform.m_location + uniform.m_sizeInBytes;
            while (uniformBytes.size() < bytes)
            {
                uniformBytes.push_back(0);
            }
        }
    }

    void VKShaderProgram::bindUniforms()
    {
        // update uniform VkBuffer by memory
        createVkUniformBuffer();

        // set uniforms
        createVkDescriptorAndPipelineLayouts();
    }

    void VKShaderProgram::bindRenderable(Renderable* renderable)
    {

    }

    const spirv_cross::ShaderResources VKShaderProgram::getSpirvShaderResources(ShaderType type)
    {
        return type == ShaderType::VS ? m_vertexShaderCompiler->get_shader_resources() : m_fragmentShaderCompiler->get_shader_resources();
    }

    const spirv_cross::Compiler* VKShaderProgram::getSpirvShaderCompiler(ShaderType type)
    {
        return type == ShaderType::VS ? m_vertexShaderCompiler : m_fragmentShaderCompiler;
    }
}