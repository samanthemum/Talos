#include "Descriptors.h"

namespace vkInit {

	vk::DescriptorSetLayout makeDescriptorSetLayout(vk::Device device, const DescriptorSetLayoutData& inputData) {

		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(inputData.count);

		// create all bindings
		for (int i = 0; i < inputData.count; i++) {
			vk::DescriptorSetLayoutBinding binding;
			binding.descriptorType = inputData.types[i];
			binding.binding = inputData.indices[i];
			binding.descriptorCount = inputData.counts[i];
			binding.stageFlags = inputData.shaderStages[i];
			layoutBindings.push_back(binding);
		}

		vk::DescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.flags = vk::DescriptorSetLayoutCreateFlags();
		createInfo.bindingCount = inputData.count;
		createInfo.pBindings = layoutBindings.data();

		try {
			return device.createDescriptorSetLayout(createInfo);
		}
		catch (vk::SystemError err) {
			return nullptr;
		}
	}

	vk::DescriptorPool createDescriptorPool(vk::Device device, uint32_t size, const DescriptorSetLayoutData& bindings) {

		std::vector<vk::DescriptorPoolSize> poolSizes;

		for (int i = 0; i < bindings.count; i++) {
			vk::DescriptorPoolSize poolSize;
			poolSize.type = bindings.types[i];
			poolSize.descriptorCount = size;
			poolSizes.push_back(poolSize);
		}

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlags();
		poolInfo.maxSets = size;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		try {
			return device.createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError e) {
			std::cout << "Couldn't create descriptor pool!" << std::endl;
			return nullptr;
		}
	}

	vk::DescriptorSet allocateDescriptorSet(
		vk::Device device,
		vk::DescriptorPool descPool,
		vk::DescriptorSetLayout descLayout
	) {
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = descPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descLayout;

		try {
			return device.allocateDescriptorSets(allocInfo)[0];
		}
		catch (vk::SystemError err) {
			return nullptr;
		}
	}
}