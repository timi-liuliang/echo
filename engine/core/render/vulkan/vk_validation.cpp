#include "vk_validation.h"
#include "vk_renderer.h"

namespace Echo
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCb(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData) 
	{
		if		(msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)				{ EchoLogError("Vulkan : %s", pMsg); }
		else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)			{ EchoLogWarning("Vulkan : %s", pMsg); }
		else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT){ EchoLogWarning("Vulkan : %s", pMsg); }
		else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)		{ EchoLogInfo("Vulkan : %s", pMsg); }
		else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)				{ EchoLogDebug("Vulkan : %s", pMsg); }

		/*
		* false indicates that layer should not bail-out of an
		* API call that had validation failures. This may mean that the
		* app dies inside the driver due to invalid parameter(s).
		* That's what would happen without validation layers, so we'll
		* keep that behavior here.
		*/
		return false;
	}

	VKValidation::VKValidation()
	{

	}

	VKValidation::~VKValidation()
	{

	}

	void VKValidation::enumerateVkValidationLayers()
	{
		if (m_vkLayers.empty())
		{
			ui32 layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			m_vkLayers.resize(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, m_vkLayers.data());
		}
	}

	bool VKValidation::checkLayer(const char* layerName)
	{
		enumerateVkValidationLayers();

		// check by layer name
		for (const VkLayerProperties& layerProperty : m_vkLayers)
		{
			if (strcmp(layerName, layerProperty.layerName) == 0)
				return true;
		}

		return false;
	}

	void VKValidation::prepareVkValidationLayers(vector<const char*>::type& validationLayers)
	{
		validationLayers.clear();
		if (checkLayer("VK_LAYER_LUNARG_standard_validation"))
		{
			validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		}
		else
		{
			validationLayers.push_back("VK_LAYER_GOOGLE_threading");
			validationLayers.push_back("VK_LAYER_LUNARG_parameter_validation");
			validationLayers.push_back("VK_LAYER_LUNARG_object_tracker");
			validationLayers.push_back("VK_LAYER_LUNARG_core_validation");
			validationLayers.push_back("VK_LAYER_GOOGLE_unique_objects");
		}
	}

	void VKValidation::create()
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		// Callback addr
		PFN_vkCreateDebugReportCallbackEXT  vkCDRC = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkRenderer->getVkInstance(), "vkCreateDebugReportCallbackEXT");
		if (vkCDRC)
		{
			VkDebugReportCallbackCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
			createInfo.pfnCallback = debugCb;
			createInfo.pUserData = nullptr;

			// create
			VKDebug(vkCDRC(vkRenderer->getVkInstance(), &createInfo, nullptr, &m_vkDebugReportCB));
		}
	}

	void VKValidation::cleanup()
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());
		PFN_vkDestroyDebugReportCallbackEXT vkDDRC = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkRenderer->getVkInstance(), "vkDestroyDebugReportCallbackEXT");
		if (vkDDRC)
		{
			vkDDRC(vkRenderer->getVkInstance(), m_vkDebugReportCB, nullptr);
		}
	}
}