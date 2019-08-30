#pragma once

#include "engine/core/render/interface/shaderprogram.h"
#include "vk_render_base.h"
#include "vk_gpu_buffer.h"
#include <thirdparty/spirv-cross/spirv_cross.hpp>

namespace Echo
{
	class VKShaderProgram : public ShaderProgram
	{
        typedef VkPipelineShaderStageCreateInfo VkPipelineSSCI;

    public:
		virtual ~VKShaderProgram();

        // is valid
        bool isLinked() const { return m_isLinked; }

        // bind
        virtual void bindUniforms() override;
        virtual void bindRenderable(Renderable* renderable) override;

    public:
        // get shader stage create info
        const array<VkPipelineSSCI, 2>& getVkShaderStageCreateInfo() { return m_vkShaderStagesCreateInfo; }

        // get shader resources
        const spirv_cross::ShaderResources getSpirvShaderResources(ShaderType type);

        // get shader compiler
        const spirv_cross::Compiler* getSpirvShaderCompiler(ShaderType type);

	private:
		// create shader library
		virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;

        // create uniform buffer
        void createVkUniformBuffer();

        // create descriptor and pipeline layouts
        void createVkDescriptorAndPipelineLayouts();

        // add uniform
        void addUniform(spirv_cross::Resource& resource, ShaderType shaderType);

        // alloc uniform bytes
        void allocUniformBytes();

        // parse uniforms
        void parseUniforms();

	private:
		bool			                m_isLinked = false;
		VkShaderModule	                m_vkVertexShader;
		VkShaderModule	                m_vkFragmentShader;
        spirv_cross::Compiler*          m_vertexShaderCompiler = nullptr;
        spirv_cross::Compiler*          m_fragmentShaderCompiler = nullptr;
        array<VkPipelineSSCI, 2>        m_vkShaderStagesCreateInfo;
        vector<Byte>::type              m_vertexShaderUniformBytes;
        vector<Byte>::type              m_fragmentShaderUniformBytes;
        VKBuffer*                       m_vkVertexShaderUniformBuffer = nullptr;
        VKBuffer*                       m_vkFragmentShaderUniformBuffer = nullptr;
        VkDescriptorSetLayout           m_vkDescriptorSetLayout;
        VkPipelineLayout                m_vkPipelineLayout;
	};
}
