#include "SingleTimeCommands.h"

void vkUtilities::startJob(vk::CommandBuffer commandBuffer) {
	commandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);
}

void vkUtilities::endJob(vk::CommandBuffer commandBuffer, vk::Queue queue) {
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	queue.submit(1, &submitInfo, nullptr);
	queue.waitIdle();
}
