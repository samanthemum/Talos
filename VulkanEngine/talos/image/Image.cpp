#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"
#include "../utilities/Memory.h"
#include "../init/Descriptors.h"
#include "../utilities/SingleTimeCommands.h"

namespace vkImage {
	vk::Image makeImage(ImageInput input) {
		vk::ImageCreateInfo createInfo;
		createInfo.flags = vk::ImageCreateFlagBits() | input.createFlags;
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent = vk::Extent3D(input.width, input.height, 1);
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = input.arrayCount;
		createInfo.format = input.format;
		createInfo.tiling = input.tiling;
		createInfo.initialLayout = vk::ImageLayout::eUndefined;
		createInfo.usage = input.usage;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.samples = vk::SampleCountFlagBits::e1;

		try {
			return input.device.createImage(createInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "Couldn't create image" << std::endl;
			return nullptr;
		}
	}

	vk::DeviceMemory makeImageMemory(ImageInput input, vk::Image image) {

		vk::MemoryRequirements reqs = input.device.getImageMemoryRequirements(image);
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = reqs.size;
		allocInfo.memoryTypeIndex = vkUtilities::findMemoryTypeIndex(
			input.physicalDevice, reqs.memoryTypeBits, input.memoryFlags
		);

		try {
			vk::DeviceMemory imageMemory = input.device.allocateMemory(allocInfo);
			input.device.bindImageMemory(image, imageMemory, 0);
			return imageMemory;
		}
		catch (vk::SystemError err) {
			std::cout << "Unable to allocate image memory" << std::endl;
			return nullptr;
		}
	}

	void transitionImageLayout(ImageLayoutTransitionInput input) {
		vkUtilities::startJob(input.commandBuffer);

		vk::ImageSubresourceRange access;
		access.aspectMask = vk::ImageAspectFlagBits::eColor;
		access.baseMipLevel = 0;
		access.levelCount = 1;
		access.baseArrayLayer = 0;
		access.layerCount = input.arrayCount;

		// Create Barrier
		vk::ImageMemoryBarrier barrier;
		barrier.oldLayout = input.oldLayout;
		barrier.newLayout = input.newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = input.image;
		barrier.subresourceRange = access;

		vk::PipelineStageFlags sourceStage, dstStage;
		if (input.oldLayout == vk::ImageLayout::eUndefined) {
			barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else {
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		input.commandBuffer.pipelineBarrier(sourceStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

		vkUtilities::endJob(input.commandBuffer, input.queue);
	}

	void copyBufferToImage(BufferCopyInput input) {
		vkUtilities::startJob(input.commandBuffer);

		vk::BufferImageCopy copy;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;

		vk::ImageSubresourceLayers access;
		access.aspectMask = vk::ImageAspectFlagBits::eColor;
		access.mipLevel = 0;
		access.baseArrayLayer = 0;
		access.layerCount = input.arrayCount;
		copy.imageSubresource = access;

		copy.imageOffset = vk::Offset3D(0, 0, 0);
		copy.imageExtent = vk::Extent3D(
			input.width,
			input.height,
			1
		);

		input.commandBuffer.copyBufferToImage(input.srcBuffer, input.image, vk::ImageLayout::eTransferDstOptimal, copy);

		vkUtilities::endJob(input.commandBuffer, input.queue);
	}

	vk::ImageView makeImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType, uint32_t arrayCount) {
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.image = image;
		createInfo.viewType = viewType;

		// can manipulate these to read different chanels
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;

		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = arrayCount;
		createInfo.subresourceRange.aspectMask = aspect;


		createInfo.format = format;

		return device.createImageView(createInfo);
	}

	vk::Format findSupportedFormat(
		vk::PhysicalDevice physicalDevice,
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features
	) {

		for (vk::Format format : candidates) {
			vk::FormatProperties properties = physicalDevice.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
					return format;
			}

			if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
					return format;
			}
		}

			std::runtime_error("Cannot find suitable image format");
	}
}