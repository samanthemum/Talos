#pragma once
#include "../../stb_image.h"
#include "../config.h"

namespace vkImage {

	struct TextureInput {
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
		std::vector<std::string> filename;
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
		vk::DescriptorSetLayout layout;
		vk::DescriptorPool pool;
		vk::ImageViewType texType;
		vk::DescriptorSet set = nullptr;
		uint32_t descriptorCount = 1;
		uint32_t dstBinding;
	};

	struct ImageInput {
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
		int width, height;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags memoryFlags;
		vk::Format format;
		uint32_t arrayCount;
		vk::ImageCreateFlags createFlags;
	};

	struct ImageLayoutTransitionInput {
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
		vk::Image image;
		vk::ImageLayout oldLayout, newLayout;
		uint32_t arrayCount;
		vk::ImageAspectFlagBits aspect = vk::ImageAspectFlagBits::eColor;
	};

	struct BufferCopyInput {
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
		vk::Buffer srcBuffer;
		vk::Image image;
		int width, height;
		uint32_t arrayCount;
	};

	vk::Image makeImage(ImageInput input, vk::ImageLayout layout = vk::ImageLayout::eUndefined);
	vk::DeviceMemory makeImageMemory(ImageInput input, vk::Image image);
	void transitionImageLayout(ImageLayoutTransitionInput input);
	void copyBufferToImage(BufferCopyInput input);
	vk::ImageView makeImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType, uint32_t arrayCount);
	vk::Format findSupportedFormat(
		vk::PhysicalDevice physicalDevice,
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features
	);
}