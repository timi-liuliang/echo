#include "vk_ray_tracer.h"
#include "vk_renderer.h"
#include "vk_mapping.h"

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

	bool VKRayTracer::createBottomLevelStructure(MeshPtr mesh)
	{
#ifdef ECHO_RAYTRACING	
		if (mesh->isValid() && mesh->getTopologyType() == Mesh::TopologyType::TT_TRIANGLELIST)
		{
			BlasInput blasInput;
			if (MeshToBlasInput(mesh, blasInput))
			{
				return true;
			}
		}
#endif

		return false;
	}

	bool VKRayTracer::MeshToBlasInput(MeshPtr mesh, BlasInput& blasInput)
	{
		VkAccelerationStructureGeometryTrianglesDataKHR triangles = {};
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = getVkBufferDeviceAddress(mesh->getVertexBuffer());
		triangles.vertexStride = mesh->getVertexStride();
		triangles.indexType = VKMapping::mapIndexType(mesh->getIndexStride());
		triangles.indexData.deviceAddress = getVkBufferDeviceAddress(mesh->getIndexBuffer());
		triangles.maxVertex = mesh->getVertexCount();

		VkAccelerationStructureGeometryKHR asGeom = {};
		asGeom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeom.pNext = nullptr;
		asGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		asGeom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		asGeom.geometry.triangles = triangles;

		VkAccelerationStructureBuildRangeInfoKHR offset;
		offset.firstVertex = 0;
		offset.primitiveCount = mesh->getPrimitiveCount();
		offset.primitiveOffset = 0;
		offset.transformOffset = 0;

		blasInput.m_asGeometry.emplace_back(asGeom);
		blasInput.m_asBuildRangeInfo.emplace_back(offset);

		return true;
	}

	VkDeviceAddress VKRayTracer::getVkBufferDeviceAddress(GPUBuffer* buffer)
	{
		VKBuffer* vkBuffer = dynamic_cast<VKBuffer*>(buffer);

		VkBufferDeviceAddressInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.pNext = nullptr;
		info.buffer = vkBuffer->getVkBuffer();

		return vkGetBufferDeviceAddress(VKRenderer::instance()->getVkDevice(), &info);
	}
}