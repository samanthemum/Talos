#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <set>
#include <string>
#include <optional>
#include "QueueFamilies.h"

namespace vkInit {

	bool checkDeviceExtensionSupport(
		const vk::PhysicalDevice& device,
		const std::vector<const char*>& extensionNames,
		const bool& debug = false
	)
	{
		// get set of extension names
		std::set<std::string> requiredExtensions(extensionNames.begin(), extensionNames.end());

		for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
			if (debug) {
				std::cout << "Device supports " << extension.extensionName << std::endl;
			}

			// delete from extensions
			requiredExtensions.erase(extension.extensionName);
		}

		// if whole set is gone, we have all required extensions!
		return requiredExtensions.empty();
	}

	bool isSuitable(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions, bool debug = false) {
		if (debug) {
			std::cout << "We are requesting the following " << requestedExtensions.size() << " device extensions" << std::endl;
		
			for (const char* extension : requestedExtensions) {
				std::cout << "\t" << extension << std::endl;
			}
		}

		return checkDeviceExtensionSupport(device, requestedExtensions, debug);
	}

	vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance, const std::vector<const char*>& requestedExtensions, bool debug = false) {
		if (debug) {
			std::cout << "Choosing physical device" << std::endl;
		}

		std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();
		if (debug) {
			std::cout << "Number of physical devices: " << availableDevices.size() << std::endl;
		}

		for (vk::PhysicalDevice device : availableDevices)
		{
			if (debug)
			{
				logDeviceProperties(device);
			}

			if (isSuitable(device, requestedExtensions, debug))
			{
				if (debug)
				{
					std::cout << "Suitable device found" << std::endl;
				}
				return device;
			}

		}

		// if it gets this far, no extensions are needed
		if (debug)
		{
			std::cout << "No suitable device found!" << std::endl;
		}
		return nullptr;
	}

	

	vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debug = false) {

		// need our queue families
		vkUtilities::QueueFamilyIndices queueFamilyIndices = vkUtilities::findQueueFamilies(physicalDevice, surface, debug);
		std::vector<uint32_t> uniqueIndices;

		// add uinque indices
		uniqueIndices.push_back(queueFamilyIndices.graphicsFamily.value());
		if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
			uniqueIndices.push_back(queueFamilyIndices.presentFamily.value());
		}
		float queuePriority = 1.0f;

		// create more dynamic information
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		for (uint32_t index : uniqueIndices) {
			queueCreateInfos.push_back(
				vk::DeviceQueueCreateInfo(
					vk::DeviceQueueCreateFlags(),
					index,
					1,
					&queuePriority
				)
			);
		}

		// request device extensions
		std::vector<const char*> deviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// define device features
		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
		// can enable features as needed here

		// set up enabled layers and extensions
		std::vector<const char*> enabledLayers;
		if (debug) {
			enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		}
		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			queueCreateInfos.size(), queueCreateInfos.data(),
			enabledLayers.size(), enabledLayers.data(),
			deviceExtensions.size(), deviceExtensions.data(),
			&deviceFeatures
		);

		try {
			vk::Device device = physicalDevice.createDevice(deviceInfo);
			if (debug) {
				std::cout << "Logical device has been created!" << std::endl;
			}

			return device;
		}

		catch (vk::SystemError) {
			if (debug) {
				std::cout << "Logical device creation failed" << std::endl;
			}

			return nullptr;
		}

		return nullptr;
	}

	// create a graphics queue from a physical device and logical device
	std::vector<vk::Queue> getQueues(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface, bool debug = false) {
		// graphics queue created at same time as device, query device for it
		vkUtilities::QueueFamilyIndices indices = vkUtilities::findQueueFamilies(physicalDevice, surface, debug);

		// we want default graphics queue
		vk::Queue graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
		vk::Queue presentQueue = device.getQueue(indices.presentFamily.value(), 0);

		// add both queues to vector
		std::vector<vk::Queue> queues = { graphicsQueue, presentQueue };
		return queues;
	}
	
}