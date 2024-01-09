#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// statically link the vulkan library with cpp
#include <vulkan/vulkan.hpp>
#include "SwapChainFrame.h"
#include "Scene.h"
#include "VertexCollection.h"
#include "Image.h"
#include "Texture.h"
#include "Job.h"
#include "WorkerThread.h"

class Engine {
	public:
		Engine(int width, int height, GLFWwindow* window, bool debugMode);
		Engine(int width, int height, GLFWwindow* window, bool debugMode, std::vector<const char*> extensions);
		~Engine();

		// modify requested extension
		void setRequestedExtensions(std::vector<const char*> newExtensions);
		std::vector<const char*> getRequestedExtensions();

		void render(Scene* scene);

	private:
		bool debugMode = true;

		// glfw params
		int width{ 640 };
		int height{ 480 };
		GLFWwindow* window{ nullptr };

		// Create a vulkan instance
		vk::Instance instance{ nullptr };
		vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
		void createDebugUtilsMessenger();
		vk::DispatchLoaderDynamic dldi;
		vk::SurfaceKHR surface;

		// physical device
		vk::PhysicalDevice physicalDevice;

		// logical device
		vk::Device device{ nullptr };

		// create graphics queue
		vk::Queue graphicsQueue{ nullptr };
		vk::Queue presentQueue{ nullptr };

		// swap chain
		vk::SwapchainKHR swapChain;
		std::vector<vkUtilities::SwapChainFrame> swapChainFrames;
		vk::Format swapChainFormat;
		vk::Extent2D swapChainExtent;

		// pipeline variables
		std::unordered_map<PipelineTypes, vk::PipelineLayout> pipelineLayouts;
		std::unordered_map<PipelineTypes, vk::RenderPass>renderPasses;
		std::unordered_map<PipelineTypes, vk::Pipeline> pipelines;

		// Command related
		vk::CommandPool commandPool;
		vk::CommandBuffer mainCommandBuffer;

		// Sync related
		int maxFramesInFlight, frameNumber;

		// Descriptor Objects
		std::vector<PipelineTypes> pipelineTypes = { {PipelineTypes::SKY, PipelineTypes::FORWARD} };
		std::unordered_map<PipelineTypes, vk::DescriptorSetLayout> vertexDescLayout;
		vk::DescriptorPool frameVertexDescPool;
		vk::DescriptorPool frameFragmentDescPool;
		std::unordered_map<PipelineTypes, vk::DescriptorSetLayout> fragmentDescLayout;
		std::unordered_map<PipelineTypes, vk::DescriptorSetLayout> meshDescLayout;
		vk::DescriptorPool meshDescPool;

		// Available Assets
		VertexCollection* meshes;
		std::unordered_map<std::string, vkImage::Texture*> textures;
		vkImage::Texture* skybox;
		vkJob::JobQueue workQueue;
		std::vector<std::thread> workers;

		// instance setup
		void setupVulkanInstance();

		// device setup
		std::vector<const char*> requestedExtensions;
		void setupDevice();
		void createSwapchain();
		void destroySwapchain();
		void recreateSwapchain();

		// pipeline setup
		void createDescriptorSetLayouts();
		void setupPipeline();
		
		void finalizeSetup();
		void createFrameBuffers();
		void createFrameResources();

		// make assets
		void makeWorkerThreads();
		void makeAssets();
		void endWorkerThreads();
		void prepareScene(vk::CommandBuffer commandBuffer);
		void prepareFrame(uint32_t imageIndex, const Scene* scene);
		void renderObjects(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount);
		void drawStandard(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
		void drawSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
};