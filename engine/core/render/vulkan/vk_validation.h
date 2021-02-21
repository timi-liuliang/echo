#pragma once

#include "vk_render_base.h"

namespace Echo
{
	class VKValidation
	{
		typedef vector<VkLayerProperties>::type		LayerProperties;

	public:
		VKValidation();
		~VKValidation();

		// prepare validation layers for vkInstance create
		void prepareVkValidationLayers(vector<const char*>::type& validationLayers);

		// check layers
		bool checkLayer(const char* layerName);

		// add layer
		bool addLayer(vector<const char*>::type& validationLayers, const char* layerName);

		// create
		void create();

		// cleanup
		void cleanup();

	private:
		// vk validation layers
		void enumerateVkValidationLayers();

	private:
		LayerProperties				m_vkLayers;
		VkDebugReportCallbackEXT	m_vkDebugReportCB;
	};
}