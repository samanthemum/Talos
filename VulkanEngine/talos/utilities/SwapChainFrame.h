#pragma once
#include "../config.h"
#include "Memory.h"
#include "../image/Image.h"
#include "../gameobjects/Light.h"
#include "../pipeline/Descriptors.h"
#include "../image/Texture.h"

namespace vkUtilities {

	struct CameraMatrices {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct CameraVectors {
		glm::vec4 forward;
		glm::vec4 right;
		glm::vec4 up;
	};

	struct LightBufferObject {
		glm::vec4 numLights;
		glm::vec4 positions[16];
		glm::vec4 colors[16];
	};

	class SwapChainFrame {

	public:
		// Devices
		vk::Device device;
		vk::PhysicalDevice physicalDevice;

		// SwapChain resources
		vk::Image image;
		vk::ImageView imageView;
		std::unordered_map<RenderPassType, vk::Framebuffer> frameBuffer;
		// TODO: Make these also maps in the future?
		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthBufferFormat;

		// G buffers
		vk::Image prepassDepthBuffer;
		vk::DeviceMemory prepassDepthBufferMemory;
		vk::ImageView prepassDepthBufferView;
		vkImage::Texture prepassDepthTexture;

		vk::Image albedoBuffer;
		vk::DeviceMemory albedoBufferMemory;
		vk::ImageView albedoBufferView;
		vkImage::Texture albedoTexture;

		vk::Image normalBuffer;
		vk::DeviceMemory normalBufferMemory;
		vk::ImageView normalBufferView;
		vkImage::Texture normalTexture;

		int width, height;


		// CommandBuffer
		vk::CommandBuffer commandBuffer;

		// Sync Resources
		vk::Fence inFlightFence;
		vk::Fence prepassFence;
		vk::Semaphore renderSemaphore;
		vk::Semaphore presentSemaphore;

		// Drawing Resources
		CameraMatrices cameraMatrixData;
		Buffer cameraMatrixBuffer;
		void* cameraMatrixWriteLocation;

		CameraVectors cameraVectorData;
		Buffer cameraVectorBuffer;
		void* cameraVectorWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		Buffer modelTransformBuffer;
		void* modelTransformWriteLocation;

		// Light Information
		LightBufferObject lightData;
		Buffer lightBuffer;
		void* lightWriteLocation;
		int numSupportedLights = 16;

		// Resource Descriptors
		vk::DescriptorBufferInfo cameraVectorDescriptor;
		vk::DescriptorBufferInfo cameraMatrixDescriptor;
		vk::DescriptorBufferInfo modelBufferDescriptor;
		vk::DescriptorBufferInfo lightBufferDescriptor;
		std::unordered_map<RenderPassType, vk::DescriptorSet> vertexDescSet;
		std::unordered_map<RenderPassType, vk::DescriptorSet> fragDescSet;

		// Descriptor sets for buffers
		vk::DescriptorSet prepassBufferDescriptorSet;

		// Write Info
		std::vector<vk::WriteDescriptorSet> writeOps;

		void createDescriptorResources();

		void createPrepassBufferTextures(vk::DescriptorPool& descPool, vk::DescriptorSetLayout& layout);
	
		void createDescriptorSets();

		void writeDescriptorSets();

		void createDepthResources();

		void createAlbedoBuffer();

		void createNormalBuffer();

		void createImageResources(vkImage::ImageInput imageInput, vk::ImageAspectFlagBits flags, vk::Image& image, vk::DeviceMemory& memory, vk::ImageView& imageView);

		void updateLightInformation(const std::vector<Light>& lights);

		void destroy();
	};
}