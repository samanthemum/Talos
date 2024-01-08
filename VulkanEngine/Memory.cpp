#include "Memory.h"
#include "SingleTimeCommands.h"

uint32_t vkUtilities::findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties) {

	vk::PhysicalDeviceMemoryProperties properties = physicalDevice.getMemoryProperties();

	// loop through all possible locations and check if the memory location is supported
	for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

		bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };
		bool sufficient{ (properties.memoryTypes[i].propertyFlags & (requestedProperties)) == requestedProperties };

		if (supported && sufficient) {
			return i;
		}
	}

	// no supported memory found
	return -1;
}

void vkUtilities::allocateBufferMemory(Buffer& buffer, const BufferInput& input) {

	vk::MemoryRequirements memoryRequirements = input.device.getBufferMemoryRequirements(buffer.buffer);

	// allocate memory to the buffer
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryTypeIndex(input.physicalDevice, memoryRequirements.memoryTypeBits, input.memoryProperties);
	buffer.bufferMemory = input.device.allocateMemory(allocInfo);
	input.device.bindBufferMemory(buffer.buffer, buffer.bufferMemory, 0);
}

void vkUtilities::copyBuffer(Buffer& sourceBuffer, Buffer& destinationBuffer, vk::DeviceSize size, vk::Queue queue, vk::CommandBuffer commandBuffer) {

	startJob(commandBuffer);

	vk::BufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	commandBuffer.copyBuffer(sourceBuffer.buffer, destinationBuffer.buffer, 1, &copyRegion);

	endJob(commandBuffer, queue);
}

Buffer vkUtilities::createBuffer(BufferInput input) {
	Buffer buffer{};

	// create and set buffer
	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.flags = vk::BufferCreateFlags();
	bufferCreateInfo.size = input.size;
	bufferCreateInfo.usage = input.usage;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	buffer.buffer = input.device.createBuffer(bufferCreateInfo);

	// Allocate buffer memory
	allocateBufferMemory(buffer, input);

	return buffer;
}