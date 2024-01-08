#pragma once
#include "config.h"

namespace vkUtilities {

	uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);

	void allocateBufferMemory(Buffer& buffer, const BufferInput& input);

	Buffer createBuffer(BufferInput input);

	void copyBuffer(Buffer& sourceBuffer, Buffer& destinationBuffer, vk::DeviceSize size, vk::Queue queue, vk::CommandBuffer commandBuffer);
}