#pragma once
#include "../config.h"
#include "../utilities/SwapChainFrame.h"

namespace vkInit {
	struct frameBufferInput {
		vk::Device device;
		std::unordered_map<PipelineTypes, vk::RenderPass> renderpass;
		vk::Extent2D swapChainExtent;
	};

	void makeFrameBuffer(frameBufferInput fbInput, std::vector<vkUtilities::SwapChainFrame>& frames, bool debug = true) {
		// Sky
		for (int i = 0; i < frames.size(); i++) {
			std::vector<vk::ImageView> attachments = {
				frames[i].imageView,
			};

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = fbInput.renderpass[PipelineTypes::SKY];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = fbInput.swapChainExtent.width;
			framebufferInfo.height = fbInput.swapChainExtent.height;
			framebufferInfo.layers = 1;

			try {
				frames[i].frameBuffer[PipelineTypes::SKY] = fbInput.device.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError err) {
				if (debug) {
					std::cout << "Failed to create frame buffer for frame " << i << std::endl;
				}
			}
		}

		// Forward
		for (int i = 0; i < frames.size(); i++) {
			std::vector<vk::ImageView> attachments = {
				frames[i].imageView,
				frames[i].depthBufferView
			};

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = fbInput.renderpass[PipelineTypes::FORWARD];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = fbInput.swapChainExtent.width;
			framebufferInfo.height = fbInput.swapChainExtent.height;
			framebufferInfo.layers = 1;

			try {
				frames[i].frameBuffer[PipelineTypes::FORWARD] = fbInput.device.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError err) {
				if (debug) {
					std::cout << "Failed to create frame buffer for frame " << i << std::endl;
				}
			}
		}

		// Deferred
		for (int i = 0; i < frames.size(); i++) {
			std::vector<vk::ImageView> attachments = {
				frames[i].albedoBufferView,
				frames[i].normalBufferView,
				frames[i].depthBufferView
			};

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = fbInput.renderpass[PipelineTypes::DEFERRED];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = fbInput.swapChainExtent.width;
			framebufferInfo.height = fbInput.swapChainExtent.height;
			framebufferInfo.layers = 1;

			try {
				frames[i].frameBuffer[PipelineTypes::DEFERRED] = fbInput.device.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError err) {
				if (debug) {
					std::cout << "Failed to create frame buffer for frame " << i << std::endl;
				}
			}
		}
	}
}