#include "vk_shader_program.h"
#include "vk_renderer.h"
#include "vk_mapping.h"
#include "base/glslcc/glsl_cross_compiler.h"

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
            m_vkShaderStagesCreateInfo[0].flags = 0;
            m_vkShaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            m_vkShaderStagesCreateInfo[0].module = m_vkVertexShader;
            m_vkShaderStagesCreateInfo[0].pName = "main";

            m_vkShaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_vkShaderStagesCreateInfo[1].flags = 0;
            m_vkShaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            m_vkShaderStagesCreateInfo[1].module = m_vkFragmentShader;
            m_vkShaderStagesCreateInfo[1].pName = "main";

            if (parseUniforms())
            {
				createVkDescriptorSetLayout(ShaderType::VS);
				createVkDescriptorSetLayout(ShaderType::FS);
                createVkPipelineLayout();
            }
        }

        return m_isLinked;
    }

    void VKShaderProgram::createVkUniformBuffer(UniformsInstance& uniformsInstance)
    {
        // this is not good here, reuse VkBuffer afterwards
        EchoSafeDelete(uniformsInstance.m_vkVertexShaderUniformBuffer, VKBuffer);
        EchoSafeDelete(uniformsInstance.m_vkFragmentShaderUniformBuffer, VKBuffer);

        Buffer vertUniformBuffer(m_vertexShaderUniformBytes.size(), m_vertexShaderUniformBytes.data(), false);
        uniformsInstance.m_vkVertexShaderUniformBuffer = EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_UNIFORM, GPUBuffer::GBU_DYNAMIC, vertUniformBuffer));

        Buffer fragmentUniformBuffer(m_fragmentShaderUniformBytes.size(), m_fragmentShaderUniformBytes.data(), false);
        uniformsInstance.m_vkFragmentShaderUniformBuffer = EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_UNIFORM, GPUBuffer::GBU_DYNAMIC, fragmentUniformBuffer));

        // Store information in the uniform's descriptor that is used by the descriptor set
        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::VS].buffer = uniformsInstance.m_vkVertexShaderUniformBuffer->getVkBuffer();
        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::VS].offset = 0;
        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::VS].range = uniformsInstance.m_vkVertexShaderUniformBuffer->getSize();

        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::FS].buffer = uniformsInstance.m_vkFragmentShaderUniformBuffer->getVkBuffer();
        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::FS].offset = 0;
        uniformsInstance.m_vkShaderUniformBufferDescriptors[ShaderType::FS].range = uniformsInstance.m_vkFragmentShaderUniformBuffer->getSize();
    }

    void VKShaderProgram::updateVkUniformBuffer(UniformsInstance& uniformsInstance)
    {
        if (!m_isLinked) return;

        // organize uniform bytes
        for (UniformMap::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            UniformPtr uniform = it->second;
            void* value = uniform->m_value.empty() ? uniform->getValueDefault().data() : uniform->m_value.data();
            if (value && uniform->m_type != SPT_UNKNOWN)
            {
                vector<Byte>::type& uniformBytes = uniform->m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
                if (uniform->m_type != SPT_TEXTURE)
                {
                    std::memcpy(uniformBytes.data() + uniform->m_location, value, uniform->m_sizeInBytes * sizeof(Byte));
                }
                else
                {

                }
            }
        }

        if (!uniformsInstance.m_vkVertexShaderUniformBuffer ||
            !uniformsInstance.m_vkFragmentShaderUniformBuffer ||
            uniformsInstance.m_vkVertexShaderUniformBuffer->getSize() != m_vertexShaderUniformBytes.size() ||
            uniformsInstance.m_vkFragmentShaderUniformBuffer->getSize() != m_fragmentShaderUniformBytes.size())
        {
            createVkUniformBuffer(uniformsInstance);
            createVkDescriptorSet(uniformsInstance);
        }

        Buffer vertUniformBuffer(m_vertexShaderUniformBytes.size(), m_vertexShaderUniformBytes.data(), false);
        uniformsInstance.m_vkVertexShaderUniformBuffer->updateData(vertUniformBuffer);

        Buffer fragmentUniformBuffer(m_fragmentShaderUniformBytes.size(), m_fragmentShaderUniformBytes.data(), false);
        uniformsInstance.m_vkFragmentShaderUniformBuffer->updateData( fragmentUniformBuffer);
    }

    void VKShaderProgram::createVkDescriptorSet(UniformsInstance& uniformsInstance)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool =VKRenderer::instance()->getVkDescriptorPool();
        allocInfo.descriptorSetCount = m_vkDescriptorSetLayouts.size();
        allocInfo.pSetLayouts = m_vkDescriptorSetLayouts.data();

        VkResult result = vkAllocateDescriptorSets(VKRenderer::instance()->getVkDevice(), &allocInfo, &uniformsInstance.m_vkDescriptorSets[0]);
        if (VK_SUCCESS == result)
        {
            // Update the descriptor set determining the shader binding points
            // For every binding point used in a shader there needs to be one
            // descriptor set matching that binding point
            vector<VkWriteDescriptorSet>::type writeDescriptorSets;
            for (size_t i = 0; i < uniformsInstance.m_vkShaderUniformBufferDescriptors.size(); i++)
            {
                if (uniformsInstance.m_vkShaderUniformBufferDescriptors[i].buffer)
                {
					// Binding 0 : Uniform buffer
					VkWriteDescriptorSet writeDescriptorSet;
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSet.pNext = nullptr;
					writeDescriptorSet.dstSet = uniformsInstance.m_vkDescriptorSets[i];
                    writeDescriptorSet.dstBinding = 0;
                    writeDescriptorSet.dstArrayElement = 0;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    writeDescriptorSet.pImageInfo = nullptr;
					writeDescriptorSet.pBufferInfo = &uniformsInstance.m_vkShaderUniformBufferDescriptors[i];
                    writeDescriptorSet.pTexelBufferView = nullptr;

					writeDescriptorSets.emplace_back(writeDescriptorSet);
                }
            }

            vkUpdateDescriptorSets(VKRenderer::instance()->getVkDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
        }
        else
        {
            EchoLogError("vulkan set descriptor set failed.");
        }
    }

    void VKShaderProgram::createVkDescriptorSetLayout(ShaderType type)
    {
        VkDescriptorSetLayoutBinding layoutBindings;
        layoutBindings.binding = 0;
        layoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings.descriptorCount = 1;
        layoutBindings.stageFlags = type == ShaderType::VS ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings.pImmutableSamplers = nullptr;

        // create a descriptor set layout based on layout bindings
        VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
        dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslCreateInfo.pNext = nullptr;
        dslCreateInfo.flags = 0;
        dslCreateInfo.bindingCount = 1;
        dslCreateInfo.pBindings = &layoutBindings;

        VKDebug(vkCreateDescriptorSetLayout(VKRenderer::instance()->getVkDevice(), &dslCreateInfo, nullptr, &m_vkDescriptorSetLayouts[type]));
    }

    void VKShaderProgram::createVkPipelineLayout()
    {
        VkPipelineLayoutCreateInfo plCreateInfo = {};
        plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plCreateInfo.pNext = nullptr;
        plCreateInfo.flags = 0;
		plCreateInfo.setLayoutCount = m_vkDescriptorSetLayouts.size();
		plCreateInfo.pSetLayouts = m_vkDescriptorSetLayouts.data();
        plCreateInfo.pushConstantRangeCount = 0;
        plCreateInfo.pPushConstantRanges = nullptr;

        VKDebug(vkCreatePipelineLayout(VKRenderer::instance()->getVkDevice(), &plCreateInfo, nullptr, &m_vkPipelineLayout));
    }

    // https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/4-Using-spir-v-with-spirv-cross.pdf
    bool VKShaderProgram::parseUniforms()
    {
        m_uniforms.clear();

        // vertex uniforms
        SPIRV_CROSS_NAMESPACE::ShaderResources vsResources = m_vertexShaderCompiler->get_shader_resources();
        {
			for (SPIRV_CROSS_NAMESPACE::Resource& resource : vsResources.uniform_buffers)
				addUniform(resource, ShaderType::VS);
        }

        // fragment uniforms
        SPIRV_CROSS_NAMESPACE::ShaderResources fsResources = m_fragmentShaderCompiler->get_shader_resources();
        {
			for (SPIRV_CROSS_NAMESPACE::Resource& resource : fsResources.uniform_buffers)
				addUniform(resource, ShaderType::FS);

			for (SPIRV_CROSS_NAMESPACE::Resource& resource : fsResources.sampled_images)
				addUniform(resource, ShaderType::FS);
        }

        allocUniformBytes();

        return !m_uniforms.empty();
    }

    void VKShaderProgram::addUniform(spirv_cross::Resource& resource, ShaderType shaderType)
    {
        spirv_cross::Compiler* compiler = shaderType == ShaderType::VS ? m_vertexShaderCompiler : m_fragmentShaderCompiler;
        const spirv_cross::SPIRType& type = compiler->get_type(resource.base_type_id);
        if (type.basetype == spirv_cross::SPIRType::SampledImage)
        {
			Uniform* desc = EchoNew(Uniform);
			desc->m_name = resource.name.c_str();
			desc->m_shader = shaderType;
			desc->m_type = VKMapping::mapUniformType(type);
			desc->m_count = 1;
            desc->m_sizeInBytes = 4;
            desc->m_location = 0;// compiler->type_struct_member_offset(type, i);
			m_uniforms[desc->m_name] = desc;
        }
        else
        {
			size_t memberCount = type.member_types.size();
			for (size_t i = 0; i < memberCount; i++)
			{
				Uniform* desc = EchoNew(Uniform);
				desc->m_name = compiler->get_member_name(type.self, i);
				desc->m_shader = shaderType;
				desc->m_sizeInBytes = compiler->get_declared_struct_member_size(type, i);
				desc->m_type = VKMapping::mapUniformType(compiler->get_type(type.member_types[i]));
				desc->m_count = desc->m_sizeInBytes / mapUniformTypeSize(desc->m_type);
				desc->m_location = compiler->type_struct_member_offset(type, i);
				m_uniforms[desc->m_name] = desc;
			}
        }
    }

    void VKShaderProgram::allocUniformBytes()
    {
        m_vertexShaderUniformBytes.clear();
        m_fragmentShaderUniformBytes.clear();

        for (auto& it : m_uniforms)
        {
            UniformPtr uniform = it.second;
            vector<Byte>::type& uniformBytes = uniform->m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
            i32 bytes = uniform->m_location + uniform->m_sizeInBytes;
            while (uniformBytes.size() < bytes)
            {
                uniformBytes.emplace_back(0);
            }
        }
    }

    void VKShaderProgram::bindUniforms(VkCommandBuffer& vkCommandbuffer, UniformsInstance& uniformsInstance)
    {
        // update uniform VkBuffer by memory
        updateVkUniformBuffer(uniformsInstance);

        // Bind descriptor sets describing shader binding points
        vkCmdBindDescriptorSets(vkCommandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, uniformsInstance.m_vkDescriptorSets.size(), uniformsInstance.m_vkDescriptorSets.data(), 0, nullptr);
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