#pragma once

#include "base/ray_tracer.h"
#include "base/mesh/mesh.h"
#include "vk_render_base.h"
#include "vk_gpu_buffer.h"

namespace Echo
{
	// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/
	class VKRayTracer : public RayTracer
	{
	public:
		// Acceleraton structure
		struct AccelKHR
		{
			VkAccelerationStructureKHR	m_accel = VK_NULL_HANDLE;
			VKBuffer*					m_buffer = nullptr;
		};

		// Inputs used to build Bottom-level acceleration structure
		struct BlasInput
		{
			vector<VkAccelerationStructureGeometryKHR>::type		m_asGeometry;
			vector<VkAccelerationStructureBuildRangeInfoKHR>::type	m_asBuildRangeInfo;
			VkBuildAccelerationStructureFlagsKHR					m_flags = 0;
		};

	public:
		VKRayTracer();
		virtual ~VKRayTracer();

		// Initialize
		void init(VkPhysicalDevice vkPhysicalDevice);

		// Build bottom level acceleration structure
		bool createBottomLevelStructure(MeshPtr mesh);

	private:
		// Mesh to BlasInput
		bool MeshToBlasInput(MeshPtr mesh, BlasInput& blasInput);

		// Get vulkan device address of VkBuffer
		VkDeviceAddress getVkBufferDeviceAddress(GPUBuffer* buffer);

	private:
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_vkRaytracingPipelineProperties = {};
		VkPhysicalDeviceProperties2						m_vkPhysicalDeviceProperties2;
		vector<AccelKHR>::type							m_blas;
		AccelKHR										m_tlas;
	};
}