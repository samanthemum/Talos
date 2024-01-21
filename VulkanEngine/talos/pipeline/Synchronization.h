#pragma once
#include "../config.h"

namespace vkInit {

	vk::Semaphore makeSemaphore(vk::Device device, bool debug = false) {
		vk::SemaphoreCreateInfo info = {};
		info.flags = vk::SemaphoreCreateFlags();

		try {
			return device.createSemaphore(info);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create sempahore" << std::endl;
			}
			return nullptr;
		}
	}

	vk::Fence makeFence(vk::Device device, bool debug = false) {
		vk::FenceCreateInfo info = {};
		info.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try {
			return device.createFence(info);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create fence" << std::endl;
			}
			return nullptr;
		}
	}

}