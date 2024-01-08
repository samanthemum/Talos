#pragma once
#include "config.h"
#include "QueueFamilies.h"
#include "Logging.h"
#include "SwapChainFrame.h";

namespace vkInit {
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capablities;

		// supported pixel format
		std::vector<vk::SurfaceFormatKHR> formats;

		// algorithm used for selecting images to present
		std::vector<vk::PresentModeKHR> presentModes;
	};

	

	struct SwapChainBundle {
		vk::SwapchainKHR swapchain;
		std::vector<vkUtilities::SwapChainFrame> frames;
		vk::Format format;
		vk::Extent2D extent;
	};

	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug = false) {

		SwapChainSupportDetails support;
		support.capablities = device.getSurfaceCapabilitiesKHR(surface);

		if (debug) {
			std::cout << "Swap Chain cabpabilities: " << std::endl;
			std::cout << "Minimum image count is " << support.capablities.minImageCount << std::endl;
			std::cout << "Maximum image count is " << support.capablities.maxImageCount << std::endl;

			// extent information
			std::cout << std::endl << std::endl;
			std::cout << "Current extent width is " << support.capablities.currentExtent.width << std::endl;
			std::cout << "Current extent height is " << support.capablities.currentExtent.height << std::endl;
			std::cout << "Max extent width is " << support.capablities.maxImageExtent.width << std::endl;
			std::cout << "Max extent height is " << support.capablities.maxImageExtent.height << std::endl;
			std::cout << "Min extent width is " << support.capablities.minImageExtent.width << std::endl;
			std::cout << "Min extent height is " << support.capablities.minImageExtent.height << std::endl;

			// max array
			std::cout << "Max array layers is " << support.capablities.maxImageArrayLayers << std::endl;

			// transform support
			std::cout << "Supported transforms" << std::endl;
			std::vector<std::string> logList = vkInit::logTransformBit(support.capablities.supportedTransforms);
			for (std::string log : logList) {
				std::cout << "\t" << log << std::endl;
			}

			// transform support
			std::cout << "Supported alpha transforms" << std::endl;
			logList = vkInit::log_alpha_composite_bits(support.capablities.supportedCompositeAlpha);
			for (std::string log : logList) {
				std::cout << "\t" << log << std::endl;
			}

			// image usage support
			std::cout << "Supported image usage" << std::endl;
			logList = vkInit::log_image_usage_bits(support.capablities.supportedUsageFlags);
			for (std::string log : logList) {
				std::cout << "\t" << log << std::endl;
			}
		}

		// surface formats
		support.formats = device.getSurfaceFormatsKHR(surface);

		if (debug) {
			for (vk::SurfaceFormatKHR format : support.formats) {
				std::cout << "Supported pixel format" << vk::to_string(format.format) << std::endl;
				std::cout << "Supported color spaces" << vk::to_string(format.colorSpace) << std::endl;
			}
		}

		// supported present modes
		support.presentModes = device.getSurfacePresentModesKHR(surface);
		if (debug) {
			for (vk::PresentModeKHR mode : support.presentModes) {
				std::string log = log_present_mode(mode);
				std::cout << "Present mode " << log << std::endl;
			}
		}

		return support;
	}

	// choose desired parameters for swap chain
	// TODO: allow engine to specify desired format
	vk::SurfaceFormatKHR chooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> formats) {

		// check preferred format
		for (vk::SurfaceFormatKHR format : formats) {
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return format;
			}
		}

		return formats[0];
	}

	// TODO: allow engine to specify desired format
	vk::PresentModeKHR chooseSwapChainPresentMode(std::vector<vk::PresentModeKHR> modes) {

		// check preferred format
		for (vk::PresentModeKHR mode : modes) {
			if (mode == vk::PresentModeKHR::eMailbox) {
				return mode;
			}
		}

		// system is guaranteed fifo
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D chooseSwapChainExtent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width, height };

			extent.width = std::min(capabilities.maxImageExtent.width, std::max(width, capabilities.minImageExtent.width));
			extent.height = std::min(capabilities.maxImageExtent.height, std::max(height, capabilities.minImageExtent.height));
			return extent;
		}
	}

	SwapChainBundle createSwapChain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, int width, int height, bool debug = false) {
		SwapChainSupportDetails support = querySwapChainSupport(physicalDevice, surface, debug);

		vk::SurfaceFormatKHR format = chooseSwapChainSurfaceFormat(support.formats);
		vk::PresentModeKHR mode = chooseSwapChainPresentMode(support.presentModes);
		vk::Extent2D extent = chooseSwapChainExtent(width, height, support.capablities);

		// select how many images will be on the swapchain
		uint32_t imageCount = std::min(support.capablities.maxImageCount, support.capablities.minImageCount + 1);

		// create info for swap chain
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(),
			surface,
			imageCount,
			format.format,
			format.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment
		);

		vkUtilities::QueueFamilyIndices indices = vkUtilities::findQueueFamilies(physicalDevice, surface, debug);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		// two separate queues in use
		if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capablities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = mode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		SwapChainBundle bundle{};
		try {
			bundle.swapchain = logicalDevice.createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("Failed to create swapchain");
		}

		// create frames 
		std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
		bundle.frames.resize(images.size());
		for (size_t i = 0; i < images.size(); i++) {
			bundle.frames[i].imageView = vkImage::makeImageView(logicalDevice, images[i], format.format, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D, 1);
			bundle.frames[i].image = images[i];
		}
		
		bundle.format = format.format;
		bundle.extent = extent;

		return bundle;
	}

	
};