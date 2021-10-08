#include "vk_ray_tracer.h"

namespace Echo
{
	VKRayTracer::VKRayTracer()
	{

	}

	VKRayTracer::~VKRayTracer()
	{

	}

	void VKRayTracer::init(VkPhysicalDevice vkPhysicalDevice)
	{
#ifdef ECHO_RAYTRACING	
		m_vkRaytracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

		m_vkPhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_vkPhysicalDeviceProperties2.pNext = &m_vkRaytracingPipelineProperties;
		vkGetPhysicalDeviceProperties2(vkPhysicalDevice, &m_vkPhysicalDeviceProperties2);
#endif
	}

	void VKRayTracer::createBottomLevelStructure(MeshPtr mesh)
	{
		ui32 vertexCount = mesh->getVertexCount();
		ui32 indexCount = mesh->getIndexCount();
	}
}