#pragma once

#include "base/ray_tracer.h"
#include "base/mesh/mesh.h"
#include "vk_render_base.h"

namespace Echo
{
	// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/
	class VKRayTracer : public RayTracer
	{
	public:
		VKRayTracer();
		virtual ~VKRayTracer();

		// Initialize
		void init(VkPhysicalDevice vkPhysicalDevice);

		// Build bottom level acceleration structure
		void createBottomLevelStructure(MeshPtr mesh);

	private:
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_vkRaytracingPipelineProperties = {};
		VkPhysicalDeviceProperties2						m_vkPhysicalDeviceProperties2;
	};
}