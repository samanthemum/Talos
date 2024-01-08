#pragma once
#include "config.h"
#include "QueueFamilies.h"

namespace vkInit{

	struct commandBufferInput {
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<vkUtilities::SwapChainFrame>& frames;
	};

	vk::CommandPool makeCommandPool(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debug = false) {
		vkUtilities::QueueFamilyIndices indices = vkUtilities::findQueueFamilies(physicalDevice, surface, debug);

		vk::CommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();

		try {
			return device.createCommandPool(commandPoolInfo);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create command pool " << std::endl;
			}

			return nullptr;
		}
	}

	void makeFrameCommandBuffers(commandBufferInput cbInput, bool debug = false) {

		// create allocation info
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = cbInput.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		for (int i = 0; i < cbInput.frames.size(); i++) {
			try {
				cbInput.frames[i].commandBuffer = cbInput.device.allocateCommandBuffers(allocInfo)[0];
			}
			catch (vk::SystemError err) {
				if (debug) {
					std::cout << "Failed to create command buffer " << i << std::endl;
				}
			}
		}

	}

	vk::CommandBuffer makeCommandBuffer(commandBufferInput cbInput, bool debug = false) {

		// create allocation info
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = cbInput.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		try {
			if (debug) {
				std::cout << "Created primary command buffer " << std::endl;
			}
			return cbInput.device.allocateCommandBuffers(allocInfo)[0];
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create primary buffer " << std::endl;
			}

			return nullptr;
		}

	}
}