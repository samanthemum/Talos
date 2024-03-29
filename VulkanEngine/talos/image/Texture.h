#pragma once
#include "../../stb_image.h"
#include "../config.h"
#include "../image/Image.h"

namespace vkImage {

	class Texture {
	public:
		Texture() {};

		void load(TextureInput texInput);

		void load(TextureInput input, vk::ImageView imageView);

		void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t set);

		void destroyImage();

		void destroySampler();

	private:
		int width, height, channels;
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
		std::vector<std::string> filenames;

		// array to data
		stbi_uc** pixels;

		// Resources
		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;
		vk::Sampler sampler;

		// Resource Descriptors
		vk::DescriptorSetLayout layout;
		vk::DescriptorSet descSet;
		vk::DescriptorPool descPool;

		// Command Handles
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;

		// Texture Type
		vk::ImageViewType textureType;

		void load();
		void populate();
		void makeView();
		void makeSampler();
		void makeDescriptorSet(uint32_t binding, uint32_t bindingCount = 1);
	};
}