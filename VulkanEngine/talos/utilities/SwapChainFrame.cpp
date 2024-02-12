#include "SwapChainFrame.h"

namespace vkUtilities {

		void SwapChainFrame::createDescriptorResources() {
			// Camera Data
			BufferInput input;
			input.device = device;
			input.physicalDevice = physicalDevice;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.size = sizeof(CameraMatrices);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			cameraMatrixBuffer = createBuffer(input);

			// map memory
			cameraMatrixWriteLocation = device.mapMemory(cameraMatrixBuffer.bufferMemory, 0, input.size);

			// Camera Data
			input.device = device;
			input.physicalDevice = physicalDevice;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.size = sizeof(CameraVectors);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			cameraVectorBuffer = createBuffer(input);

			// map memory
			cameraVectorWriteLocation = device.mapMemory(cameraVectorBuffer.bufferMemory, 0, input.size);

			// Model matrices
			input.device = device;
			input.physicalDevice = physicalDevice;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.size = 1024 * sizeof(glm::mat4);
			input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
			modelTransformBuffer = createBuffer(input);

			// map memory
			modelTransformWriteLocation = device.mapMemory(modelTransformBuffer.bufferMemory, 0, input.size);
			modelTransforms.reserve(1024);
			for (int i = 0; i < 1024; i++) {
				modelTransforms.push_back(glm::mat4(1.0f));
			}

			// Lights
			input.device = device;
			input.physicalDevice = physicalDevice;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.size = sizeof(glm::vec4) + numSupportedLights * 2 * sizeof(glm::vec4);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			lightBuffer = createBuffer(input);

			// map memory
			lightWriteLocation = device.mapMemory(lightBuffer.bufferMemory, 0, input.size);
			for (int i = 0; i < numSupportedLights; i++) {
				lightData.positions[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				lightData.colors[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}
			lightData.numLights.x = 0;

			// create buffer info
			cameraMatrixDescriptor.buffer = cameraMatrixBuffer.buffer;
			cameraMatrixDescriptor.offset = 0;
			cameraMatrixDescriptor.range = sizeof(CameraMatrices);

			cameraVectorDescriptor.buffer = cameraVectorBuffer.buffer;
			cameraVectorDescriptor.offset = 0;
			cameraVectorDescriptor.range = sizeof(CameraVectors);

			modelBufferDescriptor.buffer = modelTransformBuffer.buffer;
			modelBufferDescriptor.offset = 0;
			modelBufferDescriptor.range = 1024 * sizeof(glm::mat4);

			lightBufferDescriptor.buffer = lightBuffer.buffer;
			lightBufferDescriptor.offset = 0;
			lightBufferDescriptor.range = sizeof(glm::vec4) + numSupportedLights * 2 * sizeof(glm::vec4);
		}

		void SwapChainFrame::createPrepassBufferTextures(vk::DescriptorPool& descPool, vk::DescriptorSetLayout& layout) {
			prepassBufferDescriptorSet = vkInit::allocateDescriptorSet(device, descPool, layout);

			vkImage::TextureInput texInput;
			texInput.commandBuffer = commandBuffer;
			texInput.device = this->device;
			texInput.dstBinding = 0;
			texInput.physicalDevice = physicalDevice;
			texInput.pool = descPool;
			texInput.layout = layout;
			texInput.set = prepassBufferDescriptorSet;

			albedoTexture.load(texInput, albedoBufferView);

			texInput.dstBinding = 1;
			normalTexture.load(texInput, normalBufferView);

			texInput.dstBinding = 2;
			prepassDepthTexture.load(texInput, prepassDepthBufferView);
		}

		// TODO: Separate into write ops and a write execution?
		// TODO: this may be writing to things multiple times, yeesh
		void SwapChainFrame::createDescriptorSets() {
			
			// TODO: Break this down into write operations in a different function
			vk::WriteDescriptorSet camereaVectorWrite;
			camereaVectorWrite.dstSet = vertexDescSet[RenderPassType::SKY];
			camereaVectorWrite.dstBinding = 0;
			camereaVectorWrite.dstArrayElement = 0;
			camereaVectorWrite.descriptorCount = 1;
			camereaVectorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			camereaVectorWrite.pBufferInfo = &cameraVectorDescriptor;

			vk::WriteDescriptorSet cameraMatrixWrite;
			cameraMatrixWrite.dstSet = vertexDescSet[RenderPassType::FORWARD];
			cameraMatrixWrite.dstBinding = 0;
			cameraMatrixWrite.dstArrayElement = 0;
			cameraMatrixWrite.descriptorCount = 1;
			cameraMatrixWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraMatrixWrite.pBufferInfo = &cameraMatrixDescriptor;

			vk::WriteDescriptorSet modelWriteInfo;
			modelWriteInfo.dstSet = vertexDescSet[RenderPassType::FORWARD];
			modelWriteInfo.dstBinding = 1;
			modelWriteInfo.dstArrayElement = 0;
			modelWriteInfo.descriptorCount = 1;
			modelWriteInfo.descriptorType = vk::DescriptorType::eStorageBuffer;
			modelWriteInfo.pBufferInfo = &modelBufferDescriptor;

			vk::WriteDescriptorSet cameraMatrixWritePrepass;
			cameraMatrixWritePrepass.dstSet = vertexDescSet[RenderPassType::PREPASS];
			cameraMatrixWritePrepass.dstBinding = 0;
			cameraMatrixWritePrepass.dstArrayElement = 0;
			cameraMatrixWritePrepass.descriptorCount = 1;
			cameraMatrixWritePrepass.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraMatrixWritePrepass.pBufferInfo = &cameraMatrixDescriptor;

			vk::WriteDescriptorSet modelWriteInfoPrepass;
			modelWriteInfoPrepass.dstSet = vertexDescSet[RenderPassType::PREPASS];
			modelWriteInfoPrepass.dstBinding = 1;
			modelWriteInfoPrepass.dstArrayElement = 0;
			modelWriteInfoPrepass.descriptorCount = 1;
			modelWriteInfoPrepass.descriptorType = vk::DescriptorType::eStorageBuffer;
			modelWriteInfoPrepass.pBufferInfo = &modelBufferDescriptor;

			vk::WriteDescriptorSet lightWriteInfo;
			lightWriteInfo.dstSet = fragDescSet[RenderPassType::FORWARD];
			lightWriteInfo.dstBinding = 0;
			lightWriteInfo.dstArrayElement = 0;
			lightWriteInfo.descriptorCount = 1;
			lightWriteInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
			lightWriteInfo.pBufferInfo = &lightBufferDescriptor;

			vk::WriteDescriptorSet cameraMatrixWriteDeferredFrag;
			cameraMatrixWriteDeferredFrag.dstSet = fragDescSet[RenderPassType::DEFERRED];
			cameraMatrixWriteDeferredFrag.dstBinding = 0;
			cameraMatrixWriteDeferredFrag.dstArrayElement = 0;
			cameraMatrixWriteDeferredFrag.descriptorCount = 1;
			cameraMatrixWriteDeferredFrag.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraMatrixWriteDeferredFrag.pBufferInfo = &cameraMatrixDescriptor;

			vk::WriteDescriptorSet lightWriteInfoDeferred;
			lightWriteInfoDeferred.dstSet = fragDescSet[RenderPassType::DEFERRED];
			lightWriteInfoDeferred.dstBinding = 1;
			lightWriteInfoDeferred.dstArrayElement = 0;
			lightWriteInfoDeferred.descriptorCount = 1;
			lightWriteInfoDeferred.descriptorType = vk::DescriptorType::eUniformBuffer;
			lightWriteInfoDeferred.pBufferInfo = &lightBufferDescriptor;

			writeOps = { camereaVectorWrite, cameraMatrixWrite, modelWriteInfo, lightWriteInfo, cameraMatrixWritePrepass, modelWriteInfoPrepass, lightWriteInfoDeferred, cameraMatrixWriteDeferredFrag };
		}

		void SwapChainFrame::writeDescriptorSets() {
			device.updateDescriptorSets(writeOps, nullptr);
		}

		// Reference this when creating gbuffers, probably
		void SwapChainFrame::createDepthResources() {
			// Get depth format
			depthBufferFormat = vkImage::findSupportedFormat(physicalDevice,
				{vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint},
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment);

			vkImage::ImageInput imageInfo;
			imageInfo.device = device;
			imageInfo.physicalDevice = physicalDevice;
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
			imageInfo.memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			imageInfo.width = width;
			imageInfo.height = height;
			imageInfo.format = depthBufferFormat;
			imageInfo.arrayCount = 1;

			createImageResources(imageInfo, vk::ImageAspectFlagBits::eDepth, depthBuffer, depthBufferMemory, depthBufferView);
			createImageResources(imageInfo, vk::ImageAspectFlagBits::eDepth, prepassDepthBuffer, prepassDepthBufferMemory, prepassDepthBufferView);
		}

		void SwapChainFrame::createAlbedoBuffer() {
			vk::Format albedoBufferFormat = vkImage::findSupportedFormat(physicalDevice,
				{ vk::Format::eR8G8B8A8Unorm },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eColorAttachment);

			vkImage::ImageInput imageInfo;
			imageInfo.device = device;
			imageInfo.physicalDevice = physicalDevice;
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			imageInfo.memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			imageInfo.width = width;
			imageInfo.height = height;
			imageInfo.format = albedoBufferFormat;
			imageInfo.arrayCount = 1;

			createImageResources(imageInfo, vk::ImageAspectFlagBits::eColor, albedoBuffer, albedoBufferMemory, albedoBufferView);
		}

		void SwapChainFrame::createNormalBuffer() {
			// TODO: this format might absolutely destroy our values. f in the chat
			vk::Format normalBufferFormat = vkImage::findSupportedFormat(physicalDevice,
				{ vk::Format::eR16G16B16A16Sfloat },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eColorAttachment);

			vkImage::ImageInput imageInfo;
			imageInfo.device = device;
			imageInfo.physicalDevice = physicalDevice;
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			imageInfo.memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			imageInfo.width = width;
			imageInfo.height = height;
			imageInfo.format = normalBufferFormat;
			imageInfo.arrayCount = 1;

			createImageResources(imageInfo, vk::ImageAspectFlagBits::eColor, normalBuffer, normalBufferMemory, normalBufferView);
		}

		// Invalid formats
		void SwapChainFrame::createImageResources(vkImage::ImageInput imageInput, vk::ImageAspectFlagBits flags, vk::Image& image, vk::DeviceMemory& memory, vk::ImageView& imageView) {
			image = vkImage::makeImage(imageInput);
			memory = vkImage::makeImageMemory(imageInput, image);
			imageView = vkImage::makeImageView(device, image, imageInput.format, flags, vk::ImageViewType::e2D, 1);
		}

		void SwapChainFrame::updateLightInformation(const std::vector<Light>& lights) {
			int lightsToUpdate = lights.size() <= numSupportedLights ? lights.size() : numSupportedLights;
			lightData.numLights.x = lightsToUpdate;

			for (int i = 0; i < lightsToUpdate; i++) {
				lightData.positions[i] = glm::vec4(lights[i].position, 0.0f);
				lightData.colors[i] = glm::vec4(lights[i].color, 0.0f);
			}
		}

		void SwapChainFrame::destroy() {
			device.destroyImage(albedoBuffer);
			device.freeMemory(albedoBufferMemory);
			device.destroyImageView(albedoBufferView);
			albedoTexture.destroySampler();

			device.destroyImage(normalBuffer);
			device.freeMemory(normalBufferMemory);
			device.destroyImageView(normalBufferView);
			normalTexture.destroySampler();

			device.destroyImage(prepassDepthBuffer);
			device.freeMemory(prepassDepthBufferMemory);
			device.destroyImageView(prepassDepthBufferView);
			prepassDepthTexture.destroySampler();

			device.destroyImage(depthBuffer);
			device.freeMemory(depthBufferMemory);
			device.destroyImageView(depthBufferView);

			device.destroyImageView(imageView);
			// TODO: Better cleanup
			device.destroyFramebuffer(frameBuffer[RenderPassType::FORWARD]);
			device.destroyFramebuffer(frameBuffer[RenderPassType::SKY]);
			device.destroyFramebuffer(frameBuffer[RenderPassType::PREPASS]);
			device.destroyFramebuffer(frameBuffer[RenderPassType::DEFERRED]);
			device.destroyFence(inFlightFence);
			device.destroyFence(prepassFence);
			device.destroySemaphore(renderSemaphore);
			device.destroySemaphore(presentSemaphore);

			device.unmapMemory(cameraMatrixBuffer.bufferMemory);
			device.freeMemory(cameraMatrixBuffer.bufferMemory);
			device.destroyBuffer(cameraMatrixBuffer.buffer);

			device.unmapMemory(cameraVectorBuffer.bufferMemory);
			device.freeMemory(cameraVectorBuffer.bufferMemory);
			device.destroyBuffer(cameraVectorBuffer.buffer);

			device.unmapMemory(modelTransformBuffer.bufferMemory);
			device.freeMemory(modelTransformBuffer.bufferMemory);
			device.destroyBuffer(modelTransformBuffer.buffer);

			device.unmapMemory(lightBuffer.bufferMemory);
			device.freeMemory(lightBuffer.bufferMemory);
			device.destroyBuffer(lightBuffer.buffer);
		}
}