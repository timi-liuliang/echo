#pragma once

#include "base/shader_program.h"
#include "vk_render_base.h"
#include "vk_gpu_buffer.h"
#include <thirdparty/spirv-cross/spirv_cross.hpp>

namespace Echo
{
	class VKShaderProgram : public ShaderProgram
	{
        typedef VkPipelineShaderStageCreateInfo VkPipelineSSCI;

    public:
        // Uniform Buffer
        struct UniformsInstance
        {
			VKBuffer*                       m_vkVertexShaderUniformBuffer = nullptr;
			VKBuffer*                       m_vkFragmentShaderUniformBuffer = nullptr;
			array<VkDescriptorBufferInfo, 2>m_vkShaderUniformBufferDescriptors;
			vector<VkDescriptorSet>::type   m_vkDescriptorSets = {};
        };

    public:
		virtual ~VKShaderProgram();

        // is valid
        bool isLinked() const { return m_isLinked; }

        // bind
        void bindUniforms(VkCommandBuffer& vkCommandbuffer, UniformsInstance& uniformsInstance);

    public:
        // get shader stage create info
        const array<VkPipelineShaderStageCreateInfo, 2>& getVkShaderStageCreateInfo() { return m_vkShaderStagesCreateInfo; }

        // get shader resources
        const spirv_cross::ShaderResources getSpirvShaderResources(ShaderType type);

        // get shader compiler
        const spirv_cross::Compiler* getSpirvShaderCompiler(ShaderType type);

        // get vk pipeline layout
        VkPipelineLayout getVkPipelineLayout() { return m_vkPipelineLayout; }

	private:
		// create shader library
		virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;

        // create|update uniform buffer
        void createVkUniformBuffer(UniformsInstance& uniformData);
        void updateVkUniformBuffer(UniformsInstance& uniformData);

        // setup descriptor set
        void createVkDescriptorSet(UniformsInstance& uniformData);

        // create vk descriptor set layout
        void createVkDescriptorSetLayout(const String& uniformName, ShaderType type, VkDescriptorType descriptorType, ui32 binding);

        // create descriptor and pipeline layouts
        void createVkPipelineLayout();

        // add uniform
        void addUniform(spirv_cross::Resource& resource, ShaderType shaderType);

        // alloc uniform bytes
        void allocUniformBytes();

        // parse uniforms
        bool parseUniforms();

	private:
		bool			                    m_isLinked = false;
		VkShaderModule	                    m_vkVertexShader = VK_NULL_HANDLE;
		VkShaderModule	                    m_vkFragmentShader = VK_NULL_HANDLE;
        spirv_cross::Compiler*              m_vertexShaderCompiler = nullptr;
        spirv_cross::Compiler*              m_fragmentShaderCompiler = nullptr;
        array<VkPipelineSSCI, 2>            m_vkShaderStagesCreateInfo;
        vector<VkDescriptorSetLayout>::type m_vkDescriptorSetLayouts;
        VkPipelineLayout                    m_vkPipelineLayout = VK_NULL_HANDLE;
        vector<Byte>::type                  m_vertexShaderUniformBytes;
        vector<Byte>::type                  m_fragmentShaderUniformBytes;
	};
}
