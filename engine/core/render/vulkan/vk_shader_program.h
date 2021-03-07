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
        struct UniformsData
        {
			VKBuffer*                       m_vkVertexShaderUniformBuffer = nullptr;
			VKBuffer*                       m_vkFragmentShaderUniformBuffer = nullptr;
			array<VkDescriptorBufferInfo, 2>m_vkShaderUniformBufferDescriptors;
			array<VkDescriptorSet, 2>       m_vkDescriptorSets = {};
        };

    public:
		virtual ~VKShaderProgram();

        // is valid
        bool isLinked() const { return m_isLinked; }

        // bind
        void bindUniforms(VkCommandBuffer& vkCommandbuffer);

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
        void createVkUniformBuffer();
        void updateVkUniformBuffer();

        // setup descriptor set
        void createVkDescriptorSet();

        // create vk descriptor set layout
        void createVkDescriptorSetLayout(ShaderType type);

        // create descriptor and pipeline layouts
        void createVkPipelineLayout();

        // add uniform
        void addUniform(spirv_cross::Resource& resource, ShaderType shaderType);

        // alloc uniform bytes
        void allocUniformBytes();

        // parse uniforms
        bool parseUniforms();

	private:
		bool			                m_isLinked = false;
		VkShaderModule	                m_vkVertexShader = VK_NULL_HANDLE;
		VkShaderModule	                m_vkFragmentShader = VK_NULL_HANDLE;
        spirv_cross::Compiler*          m_vertexShaderCompiler = nullptr;
        spirv_cross::Compiler*          m_fragmentShaderCompiler = nullptr;
        array<VkPipelineSSCI, 2>        m_vkShaderStagesCreateInfo;
        array<VkDescriptorSetLayout, 2> m_vkDescriptorSetLayouts = {};
        VkPipelineLayout                m_vkPipelineLayout = VK_NULL_HANDLE;
        vector<Byte>::type              m_vertexShaderUniformBytes;
        vector<Byte>::type              m_fragmentShaderUniformBytes;
		VKBuffer*                       m_vkVertexShaderUniformBuffer = nullptr;
		VKBuffer*                       m_vkFragmentShaderUniformBuffer = nullptr;
		array<VkDescriptorBufferInfo, 2>m_vkShaderUniformBufferDescriptors;
		array<VkDescriptorSet, 2>       m_vkDescriptorSets = {};
	};
}
