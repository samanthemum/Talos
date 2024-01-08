#pragma once
#include "config.h"


namespace vkUtilities {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;

		// note: generally graphics and present queue index can be sent to the same
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug = false) {
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queueProperties = device.getQueueFamilyProperties();

		if (debug) {
			std::cout << "Device can support " << queueProperties.size() << " families." << std::endl;
		}

		int i = 0;
		for (vk::QueueFamilyProperties properties : queueProperties) {

			// find the graphics family
			if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsFamily = i;

				if (debug) {
					std::cout << "Queue family " << i << " is suitable for graphics and presenting" << std::endl;
				}
			}

			if (device.getSurfaceSupportKHR(i, surface)) {
				indices.presentFamily = i;

				if (debug) {
					std::cout << "Queue family " << i << " is suitable for presenting" << std::endl;
				}
			}

			// break if all queue families are set
			if (indices.isComplete()) {
				break;
			}


			// increase index if we haven't found anything that supports it
			i++;
		}

		return indices;
	}
};