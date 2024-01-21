#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>

namespace vkInit {

	bool isSupported(std::vector<const char*> extensions, std::vector<const char*> layers, bool debug = false) {
		// search device for supported extensions
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

		if (debug) {
			std::cout << "Supported extensions:" << std::endl;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
				std::cout << "\t" << supportedExtension.extensionName << std::endl;
			}
		}

		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
		if (debug) {
			std::cout << "Supported layers:" << std::endl;
			for (vk::LayerProperties supportedLayer : supportedLayers) {
				std::cout << "\t" << supportedLayer.layerName << std::endl;
			}
		}

		bool allSupported = true;

		for (int i = 0; i < extensions.size(); i++) {
			bool found = false;

			for (int j = 0; j < supportedExtensions.size(); j++) {
				if (strcmp(supportedExtensions[j].extensionName, extensions[i]) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				allSupported = false;
				
				if (debug) {
					std::cout << "Extension not supported: " << extensions[i] << std::endl;
				}
			}
		}

		for (int i = 0; i < layers.size(); i++) {
			bool found = false;

			for (int j = 0; j < supportedLayers.size(); j++) {
				if (strcmp(supportedLayers[j].layerName, layers[i]) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				allSupported = false;

				if (debug) {
					std::cout << "Layer not supported: " << layers[i] << std::endl;
				}
			}
		}
		
		return allSupported;
	}
	
	vk::Instance makeInstance(const char* applicationName, bool debug = false) {
		if (debug) {
			std::cout << "Making an instance . . . \n" << std::endl;
		}

		// query system for vulkan support
		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);
		if (debug) {
			std::cout << "Vulkan major version: " << VK_API_VERSION_MAJOR(version) << std::endl;
			std::cout << "Vulkan minor version: " << VK_API_VERSION_MINOR(version) << std::endl;
			std::cout << "Vulkan patch version: " << VK_API_VERSION_PATCH(version) << std::endl;
		}

		// declare the version we're going to use
		// zero out patch component to make sure we support every patch within version
		// might need to change for device compatability!
		version &= ~(0xFFFU);

		// declare application info
		vk::ApplicationInfo appInfo = vk::ApplicationInfo(
			applicationName,
			version,
			"Custom Engine",
			version,
			version
		);

		// request extensions
		uint32_t extensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);
		
		if (debug) {
			std::cout << "Extensions requested for GLFW:" << std::endl;

			for (const char* name : extensions) {
				std::cout << "\t" << name << std::endl;
			}
		}

		if (debug) {
			extensions.push_back("VK_EXT_debug_utils");
		}

		// request layers
		std::vector<const char*> layers;
		if (debug) {
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}
		
		// check support
		if (!isSupported(extensions, layers, debug)) {
			return nullptr;
		}
		// get create info for instance
		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(layers.size()),
			layers.data(),
			static_cast<uint32_t>(extensions.size()),
			extensions.data()
		);

		// create instance
		try {
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create Intance!" << std::endl;
			}

			return nullptr;
		}
	}
}