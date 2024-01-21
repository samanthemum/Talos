#include "Texture.h"
#include "../../stb_image.h"
#include "../utilities/Memory.h"
#include "../pipeline/Descriptors.h"

namespace vkImage {

	void Texture::load(TextureInput input) {
		device = input.device;
		physicalDevice = input.physicalDevice;
		filenames = input.filename;
		commandBuffer = input.commandBuffer;
		queue = input.queue;
		layout = input.layout;
		descPool = input.pool;
		textureType = input.texType;

		load();

		ImageInput imageInput;
		imageInput.device = device;
		imageInput.physicalDevice = physicalDevice;
		imageInput.arrayCount = filenames.size();
		imageInput.height = height;
		imageInput.width = width;
		imageInput.tiling = vk::ImageTiling::eOptimal;
		imageInput.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		imageInput.memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		imageInput.format = vk::Format::eR8G8B8A8Unorm;
		if (textureType == vk::ImageViewType::eCube) {
			imageInput.createFlags = vk::ImageCreateFlagBits::eCubeCompatible;
		}

		image = makeImage(imageInput);
		imageMemory = makeImageMemory(imageInput, image);

		populate();

		for (int i = 0; i < filenames.size(); i++) {
			free(pixels[i]);
		}

		makeView();
		makeSampler();
		makeDescriptorSet();
	}

	Texture::~Texture() {
		device.freeMemory(imageMemory);
		device.destroyImage(image);
		device.destroyImageView(imageView);
		device.destroySampler(sampler);
	}

	void Texture::use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t set) {
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, set, descSet, nullptr);
	}

	void Texture::load() {
		// log errors for inappropriate sizes
		if (filenames.size() < 1) {
			std::cerr << "WARNING: No file names included in texture creation" << std::endl;
		}
		else if (filenames.size() != 1 && textureType == vk::ImageViewType::e2D) {
			std::cerr << "WARNING: Multiple files included for 2D texture" << std::endl;
		}
		else if (filenames.size() != 6 && textureType == vk::ImageViewType::eCube) {
			std::cerr << "WARNING: More or less than 6 files included for cubemap texture" << std::endl;
		}
		// initialize pixels
		pixels = new stbi_uc* [filenames.size()];
		
		for (int i = 0; i < filenames.size(); i++) {
			pixels[i] = stbi_load(filenames[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!pixels[i]) {
				std::cout << "Failed to load filename: " << filenames[i] << std::endl;
			}
		}
	}

	void Texture::populate() {
		BufferInput input;
		input.device = device;
		input.physicalDevice = physicalDevice;
		input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
		input.usage = vk::BufferUsageFlagBits::eTransferSrc;
		size_t image_size = width * height * sizeof(float);
		input.size = image_size * filenames.size();
		Buffer stagingbuffer = vkUtilities::createBuffer(input);

		for (int i = 0; i < filenames.size(); i++) {
			void* writeLocation = device.mapMemory(stagingbuffer.bufferMemory, i * image_size, image_size);
			memcpy(writeLocation, pixels[i], image_size);
			device.unmapMemory(stagingbuffer.bufferMemory);
		}

		ImageLayoutTransitionInput layoutInput;
		layoutInput.commandBuffer = commandBuffer;
		layoutInput.queue = queue;
		layoutInput.image = image;
		layoutInput.oldLayout = vk::ImageLayout::eUndefined;
		layoutInput.newLayout = vk::ImageLayout::eTransferDstOptimal;
		layoutInput.arrayCount = filenames.size();
		transitionImageLayout(layoutInput);

		// Copy to image
		BufferCopyInput copyInput;
		copyInput.commandBuffer = commandBuffer;
		copyInput.queue = queue;
		copyInput.image = image;
		copyInput.width = width;
		copyInput.height = height;
		copyInput.srcBuffer = stagingbuffer.buffer;
		copyInput.arrayCount = filenames.size();
		copyBufferToImage(copyInput);


		layoutInput.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		layoutInput.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		transitionImageLayout(layoutInput);

		device.freeMemory(stagingbuffer.bufferMemory);
		device.destroyBuffer(stagingbuffer.buffer);
	}

	void Texture::makeView() {
		imageView = makeImageView(device, 
								  image, 
								  vk::Format::eR8G8B8A8Unorm, 
								  vk::ImageAspectFlagBits::eColor,
								  textureType,
								  filenames.size());
	}

	void Texture::makeSampler() {
		// Populate input create info
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = vk::SamplerCreateFlags();
		createInfo.minFilter = vk::Filter::eNearest;
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eAlways;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.mipLodBias = 0.0f;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 0.0f;

		try {
			sampler = device.createSampler(createInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "Failed to create sampler" << std::endl;
		}
	}

	void Texture::makeDescriptorSet() {
		descSet = vkInit::allocateDescriptorSet(device, descPool, layout);

		vk::DescriptorImageInfo imageDescriptor;
		imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageDescriptor.imageView = imageView;
		imageDescriptor.sampler = sampler;

		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageDescriptor;
		device.updateDescriptorSets(descriptorWrite, nullptr);
	}
}