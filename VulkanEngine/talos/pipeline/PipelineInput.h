#pragma once
#include "../config.h"

namespace vkInit {
	struct PipelineInput {
		PipelineTypes pipelineType;
		const char* vertexShaderLocation;
		const char* fragmentShaderLocation;
		bool shouldOverWriteColor = false;
		bool depthTest = true;
		bool shouldClearDepthAttachment = false;
		std::vector<vk::Format> formats;
		vk::Format depthFormat;
		vk::Extent2D size;
		vk::VertexInputBindingDescription vertexBindingDescription;
		std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescription{};
	};
}