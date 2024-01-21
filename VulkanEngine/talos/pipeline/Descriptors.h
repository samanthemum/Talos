#pragma once
#include "../config.h"

namespace vkInit {

	struct DescriptorSetLayoutData {
		int count;
		std::vector<int> indices;
		std::vector<vk::DescriptorType> types;
		std::vector<int> counts;
		std::vector<vk::ShaderStageFlags> shaderStages;
	};

	vk::DescriptorSetLayout makeDescriptorSetLayout(vk::Device device, const DescriptorSetLayoutData& inputData);

	vk::DescriptorPool createDescriptorPool(vk::Device device, uint32_t size, const DescriptorSetLayoutData& bindings);

	vk::DescriptorSet allocateDescriptorSet(
		vk::Device device,
		vk::DescriptorPool descPool,
		vk::DescriptorSetLayout descLayout
	);
}