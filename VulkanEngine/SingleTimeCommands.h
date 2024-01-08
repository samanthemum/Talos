#pragma once
#include "config.h"

/*
	Used for starting and finishing one off jobs
*/
namespace vkUtilities {
	void startJob(vk::CommandBuffer commandBuffer);

	void endJob(vk::CommandBuffer commandBuffer, vk::Queue queue);
}