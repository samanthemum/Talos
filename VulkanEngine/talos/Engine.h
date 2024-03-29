#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// statically link the vulkan library with cpp
#include <vulkan/vulkan.hpp>
#include "utilities/SwapChainFrame.h"
#include "gameobjects/Scene.h"
#include "mesh/VertexCollection.h"
#include "image/Image.h"
#include "image/Texture.h"
#include "job/Job.h"
#include "job/WorkerThread.h"
#include "pipeline/PipelineInput.h"
#include "pipeline/Pipeline.h"
#include "gameobjects/MeshActor.h"

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
		std::unordered_map<RenderPassType, vk::PipelineLayout> pipelineLayouts;
		std::unordered_map<RenderPassType, vk::RenderPass>renderPasses;
		std::unordered_map<RenderPassType, vk::Pipeline> pipelines;

		// Command related
		vk::CommandPool commandPool;
		vk::CommandBuffer mainCommandBuffer;

		// Sync related
		int maxFramesInFlight, frameNumber;

		// Descriptor Objects
		std::vector<RenderPassType> pipelineTypes = { {RenderPassType::SKY, RenderPassType::FORWARD} };
		std::unordered_map<RenderPassType, vk::DescriptorSetLayout> vertexDescLayout;
		vk::DescriptorPool frameVertexDescPool;
		vk::DescriptorPool frameFragmentDescPool;
		vk::DescriptorPool frameVertexDescPoolDeferred;
		vk::DescriptorPool frameFragmentDescPoolDeferred;
		std::unordered_map<RenderPassType, vk::DescriptorSetLayout> fragmentDescLayout;
		std::unordered_map<RenderPassType, vk::DescriptorSetLayout> meshDescLayout;
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
		void addPipeline(vkInit::PipelineBuilder pipelineBuilder, vkInit::PipelineInput pipelineInput);
		
		void finalizeSetup();
		void createFrameBuffers();
		void createFrameResources();

		// make assets
		void makeWorkerThreads();
		void makeAssets(Scene* scene);
		void endWorkerThreads();
		void prepareScene(vk::CommandBuffer commandBuffer);
		void prepareFrame(uint32_t imageIndex, const Scene* scene);
		void renderObjects(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount);
		void renderObjectsPrepass(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount);
		void drawStandard(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
		void drawPrepass(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
		void drawDeferred(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
		void drawSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
};