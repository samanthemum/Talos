#include "Engine.h"

#include <iostream>
#include "pipeline/Instance.h"
#include "utilities/Logging.h"
#include "pipeline/Device.h"
#include "pipeline/Swapchain.h"
#include "pipeline/Pipeline.h"
#include "pipeline/Framebuffer.h"
#include "pipeline/Command.h"
#include "pipeline/Synchronization.h"
#include "pipeline/Descriptors.h"
#include "mesh/ObjMesh.h"
#include "utilities/ModelLoader.h"

Engine::Engine(int width, int height, GLFWwindow* window, bool debugMode) {
	if (debugMode) {
		std::cout << "Making the graphics engine" << std::endl;
	}

	this->width = width;
	this->height = height;
	this->window = window;
	this->debugMode = debugMode;

	setupVulkanInstance();

	std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	setRequestedExtensions(extensions);

	setupDevice();

	createDescriptorSetLayouts();

	setupPipeline();

	finalizeSetup();
}

Engine::Engine(int width, int height, GLFWwindow* window, bool debugMode, std::vector<const char*> extensions) {
	if (debugMode) {
		std::cout << "Making the graphics engine" << std::endl;
	}

	this->width = width;
	this->height = height;
	this->window = window;
	this->debugMode = debugMode;

	setupVulkanInstance();

	setRequestedExtensions(extensions);

	setupDevice();

	createDescriptorSetLayouts();

	setupPipeline();

	finalizeSetup();
}

void Engine::setupVulkanInstance() {

	instance = vkInit::makeInstance("Vulkan Engine", debugMode);
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	if (debugMode) {
		createDebugUtilsMessenger();
	}

	// takes a c style pointer
	VkSurfaceKHR c_style_surface;
	VkResult wasSurfaceSuccessful = glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface);
	if (wasSurfaceSuccessful != VK_SUCCESS) {
		if (debugMode) {
			std::cout << "Window surface creation failed" << std::endl;
			std::cout << "Error number was " << wasSurfaceSuccessful << std::endl;
		}
	}
	else if (debugMode) {
		std::cout << "Window surface creation succeeded" << std::endl;
	}

	// casts old style and casts into class
	surface = c_style_surface;
}

void Engine::createDebugUtilsMessenger() {
	std::cout << "Creating debug messenger!" << std::endl;
	debugMessenger = vkUtilities::make_debug_messenger(instance, dldi);
}

void Engine::setRequestedExtensions(std::vector<const char*> newExtensions) {
	requestedExtensions = newExtensions;
}

std::vector<const char*> Engine::getRequestedExtensions() {
	return requestedExtensions;
}

void Engine::setupDevice() {
	physicalDevice = vkInit::selectPhysicalDevice(instance, requestedExtensions, debugMode);
	device = vkInit::createLogicalDevice(physicalDevice, surface, debugMode);

	std::vector<vk::Queue> queues = vkInit::getQueues(physicalDevice, device, surface, debugMode);
	graphicsQueue = queues[0];
	presentQueue = queues[1];

	// get swap chain support
	// vkInit::querySwapChainSupport(physicalDevice, surface, true);
	createSwapchain();
}

void Engine::createSwapchain() {
	glfwGetFramebufferSize(window, &width, &height);
	vkInit::SwapChainBundle bundle = vkInit::createSwapChain(device, physicalDevice, surface, width, height, debugMode);
	swapChain = bundle.swapchain;
	swapChainFrames = bundle.frames;
	swapChainFormat = bundle.format;
	swapChainExtent = bundle.extent;
	maxFramesInFlight = static_cast<int>(swapChainFrames.size());
	frameNumber = 0;

	for (vkUtilities::SwapChainFrame& frame : swapChainFrames) {
		frame.device = device;
		frame.physicalDevice = physicalDevice;
		frame.width = swapChainExtent.width;
		frame.height = swapChainExtent.height;

		frame.createDepthResources();

		frame.createAlbedoBuffer();
		frame.createNormalBuffer();
	}
}

void Engine::destroySwapchain() {
	for (vkUtilities::SwapChainFrame& frame : swapChainFrames) {
		frame.destroy();
	}

	device.destroySwapchainKHR(swapChain);

	device.destroyDescriptorPool(frameVertexDescPool);
	device.destroyDescriptorPool(frameFragmentDescPool);
	device.destroyDescriptorPool(frameFragmentDescPoolDeferred);
	device.destroyDescriptorPool(frameVertexDescPoolDeferred);
}

void Engine::recreateSwapchain() {
	width = 0;
	height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	// destroy old resources
	destroySwapchain();

	// allocate new
	createSwapchain();

	// create frame buffers and command buffers
	createFrameBuffers();
	vkInit::commandBufferInput cbInput{ device, commandPool, swapChainFrames };
	makeFrameCommandBuffers(cbInput, debugMode);
	createFrameResources();

	// TODO: Dellocate and reallocate pipeline to fix viewport when resizing
	// setupPipeline();
}

void Engine::createDescriptorSetLayouts()  {

	// Vertex shader descriptor bindings
	vkInit::DescriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.shaderStages.push_back(vk::ShaderStageFlagBits::eVertex);
	vertexDescLayout[RenderPassType::SKY] = vkInit::makeDescriptorSetLayout(device, bindings);

	// NOTE: This will need to changed for the forward pass with deferred!
	bindings.count = 2;
	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.shaderStages.push_back(vk::ShaderStageFlagBits::eVertex);
	vertexDescLayout[RenderPassType::FORWARD] = vkInit::makeDescriptorSetLayout(device, bindings);
	vertexDescLayout[RenderPassType::PREPASS] = vkInit::makeDescriptorSetLayout(device, bindings);

	// Fragment shader descriptor bindings
	// TODO: Split between this and mesh bindings seems super duper arbitrary???
	vkInit::DescriptorSetLayoutData fragmentBindings;
	fragmentBindings.count = 1;
	fragmentBindings.indices.push_back(0);
	fragmentBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	fragmentBindings.counts.push_back(1);
	fragmentBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);
	fragmentDescLayout[RenderPassType::FORWARD] = vkInit::makeDescriptorSetLayout(device, fragmentBindings);

	// Need a second one for deferred
	fragmentBindings.count = 2;
	fragmentBindings.indices.push_back(1);
	fragmentBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	fragmentBindings.counts.push_back(1);
	fragmentBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);
	fragmentDescLayout[RenderPassType::DEFERRED] = vkInit::makeDescriptorSetLayout(device, fragmentBindings);

	vkInit::DescriptorSetLayoutData meshBindings;
	meshBindings.count = 1;
	meshBindings.indices.push_back(0);
	meshBindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshBindings.counts.push_back(1);
	meshBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);

	// TODO: Figure out a better way to do this
	meshDescLayout[RenderPassType::FORWARD] = vkInit::makeDescriptorSetLayout(device, meshBindings);
	meshDescLayout[RenderPassType::PREPASS] = vkInit::makeDescriptorSetLayout(device, meshBindings);
	meshDescLayout[RenderPassType::SKY] = vkInit::makeDescriptorSetLayout(device, meshBindings);

	// Need 3 samplers for deferred lol
	meshBindings.count = 3;
	meshBindings.indices.push_back(1);
	meshBindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);
	meshBindings.counts.push_back(1);
	meshBindings.indices.push_back(2);
	meshBindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshBindings.counts.push_back(1);
	meshBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);
	meshDescLayout[RenderPassType::DEFERRED] = vkInit::makeDescriptorSetLayout(device, meshBindings);
}

void Engine::setupPipeline() {
	vkInit::PipelineBuilder pipelineBuilder(device);

	vkInit::PipelineInput pipelineInput;

	// Sky
	pipelineInput.depthTest = false;
	pipelineInput.shouldOverWriteColor = true;
	pipelineInput.vertexShaderLocation = "Shaders/sky_vert.spv";
	pipelineInput.fragmentShaderLocation = "Shaders/sky_frag.spv";
	pipelineInput.shouldClearDepthAttachment = true;
	pipelineInput.size = swapChainExtent;
	pipelineInput.formats = { vk::Format::eR8G8B8A8Unorm };
	pipelineInput.pipelineType = RenderPassType::SKY;
	pipelineInput.imageInitialLayout = vk::ImageLayout::eUndefined;
	pipelineInput.imageFinalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	
	addPipeline(pipelineBuilder, pipelineInput);

	// Forward
	pipelineInput.pipelineType = RenderPassType::FORWARD;
	pipelineInput.depthTest = true;
	pipelineInput.shouldOverWriteColor = false;
	pipelineInput.vertexShaderLocation = "Shaders/vert.spv";
	pipelineInput.fragmentShaderLocation = "Shaders/frag.spv";
	pipelineInput.shouldClearDepthAttachment = false;
	pipelineInput.depthFormat = swapChainFrames[0].depthBufferFormat;
	pipelineInput.formats = { swapChainFormat };
	// TODO: These lines have to change for use with the deferred pass
	pipelineInput.vertexAttributeDescription = vkMesh::getPosColorAttributeDescriptions();
	pipelineInput.vertexBindingDescription = vkMesh::getPosColorBindingDescription();
	pipelineInput.imageInitialLayout = vk::ImageLayout::ePresentSrcKHR;
	pipelineInput.imageFinalLayout = vk::ImageLayout::ePresentSrcKHR;

	addPipeline(pipelineBuilder, pipelineInput);

	// Prepass
	pipelineInput.pipelineType = RenderPassType::PREPASS;
	pipelineInput.vertexShaderLocation = "Shaders/prepass_vert.spv";
	pipelineInput.fragmentShaderLocation = "Shaders/prepass_frag.spv";
	pipelineInput.formats = { vk::Format::eR8G8B8A8Unorm,  vk::Format::eR16G16B16A16Sfloat};
	pipelineInput.vertexAttributeDescription = vkMesh::getPosColorAttributeDescriptions();
	pipelineInput.vertexBindingDescription = vkMesh::getPosColorBindingDescription();
	pipelineInput.imageInitialLayout = vk::ImageLayout::eUndefined;
	pipelineInput.imageFinalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	addPipeline(pipelineBuilder, pipelineInput);

	// Deferred
	pipelineInput.pipelineType = RenderPassType::DEFERRED;
	pipelineInput.depthTest = true;
	pipelineInput.shouldOverWriteColor = false;
	pipelineInput.vertexShaderLocation = "Shaders/deferred_vert.spv";
	pipelineInput.fragmentShaderLocation = "Shaders/deferred_frag.spv";
	pipelineInput.shouldClearDepthAttachment = false;
	pipelineInput.formats = { swapChainFormat };
	pipelineInput.depthFormat = swapChainFrames[0].depthBufferFormat;
	pipelineInput.imageInitialLayout = vk::ImageLayout::eUndefined;
	pipelineInput.imageFinalLayout = vk::ImageLayout::ePresentSrcKHR;
	pipelineInput.vertexAttributeDescription = {};

	addPipeline(pipelineBuilder, pipelineInput);
}

void Engine::addPipeline(vkInit::PipelineBuilder pipelineBuilder, vkInit::PipelineInput pipelineInput) {
	pipelineBuilder.reset();

	if (pipelineInput.vertexAttributeDescription.size() > 0) {
		pipelineBuilder.specifyVertexFormat(pipelineInput.vertexBindingDescription, pipelineInput.vertexAttributeDescription);
	}

	pipelineBuilder.setColorOverwrite(pipelineInput.shouldOverWriteColor);
	pipelineBuilder.specifyVertexShader(pipelineInput.vertexShaderLocation);
	pipelineBuilder.specifyFragmentShader(pipelineInput.fragmentShaderLocation);
	pipelineBuilder.specifySwapchainExtent(pipelineInput.size);
	pipelineBuilder.clearDepthAttachment();
	if (vertexDescLayout[pipelineInput.pipelineType] != nullptr) {
		pipelineBuilder.addDescriptorSetLayout(vertexDescLayout[pipelineInput.pipelineType]);
	}

	if (fragmentDescLayout[pipelineInput.pipelineType] != nullptr) {
		pipelineBuilder.addDescriptorSetLayout(fragmentDescLayout[pipelineInput.pipelineType]);
	}

	if (meshDescLayout[pipelineInput.pipelineType] != nullptr) {
		pipelineBuilder.addDescriptorSetLayout(meshDescLayout[pipelineInput.pipelineType]);
	}

	for (int i = 0; i < pipelineInput.formats.size(); i++) {
		pipelineBuilder.addColorAttachment(pipelineInput.formats[i], i, pipelineInput.imageInitialLayout, pipelineInput.imageFinalLayout);
	}

	pipelineBuilder.setDepthTest(pipelineInput.depthTest);
	if (pipelineInput.depthTest) {
		pipelineBuilder.specifyDepthAttachment(pipelineInput.depthFormat, pipelineInput.formats.size());
	}

	vkInit::GraphicsPipelineOutBundle output = pipelineBuilder.build();

	pipelineLayouts[pipelineInput.pipelineType] = output.layout;
	renderPasses[pipelineInput.pipelineType] = output.renderPass;
	pipelines[pipelineInput.pipelineType] = output.pipeline;
}

void Engine::createFrameBuffers() {
	vkInit::frameBufferInput fbInput;
	fbInput.device = device;
	fbInput.renderpass = renderPasses;
	fbInput.swapChainExtent = swapChainExtent;
	vkInit::makeFrameBuffer(fbInput, swapChainFrames, debugMode);
}

void Engine::createFrameResources() {

	// TODO: lol rename these to be more about the descriptors they contain lmao
	vkInit::DescriptorSetLayoutData vertexBindingsForward;
	vertexBindingsForward.count = 2;
	vertexBindingsForward.types.push_back(vk::DescriptorType::eUniformBuffer);
	vertexBindingsForward.types.push_back(vk::DescriptorType::eStorageBuffer);
	frameVertexDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size() * 3), vertexBindingsForward);

	// TODO: Unused for now, delete later
	vkInit::DescriptorSetLayoutData vertexBindingsDeferred;
	vertexBindingsDeferred.count = 1;
	vertexBindingsDeferred.types.push_back(vk::DescriptorType::eUniformBuffer);
	// frameVertexDescPoolDeferred = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size()), vertexBindingsDeferred);

	vkInit::DescriptorSetLayoutData fragmentBindings;
	fragmentBindings.count = 1;
	fragmentBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	frameFragmentDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size()), fragmentBindings);

	vkInit::DescriptorSetLayoutData fragmentBindingsDeferred;
	fragmentBindingsDeferred.count = 2;
	fragmentBindingsDeferred.types.push_back(vk::DescriptorType::eUniformBuffer);
	fragmentBindingsDeferred.types.push_back(vk::DescriptorType::eUniformBuffer);
	frameFragmentDescPoolDeferred = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size()), fragmentBindingsDeferred);

	for (int i = 0; i < maxFramesInFlight; i++) {
		swapChainFrames[i].renderSemaphore = vkInit::makeSemaphore(device, debugMode);
		swapChainFrames[i].presentSemaphore = vkInit::makeSemaphore(device, debugMode);
		swapChainFrames[i].inFlightFence = vkInit::makeFence(device, debugMode);

		// has to be reset
		swapChainFrames[i].prepassFence = vkInit::makeFence(device, debugMode);
		device.resetFences(1, &swapChainFrames[i].prepassFence);
		
		// TODO: Make this easier in the future for multiple textures
		swapChainFrames[i].createDescriptorResources();
		// swapChainFrames[i].createBufferDescriptorSets(meshDescPool, meshDescLayout[PipelineTypes::DEFERRED]);

		swapChainFrames[i].vertexDescSet[RenderPassType::FORWARD] = vkInit::allocateDescriptorSet(device, frameVertexDescPool, vertexDescLayout[RenderPassType::FORWARD]);
		swapChainFrames[i].fragDescSet[RenderPassType::FORWARD] = vkInit::allocateDescriptorSet(device, frameFragmentDescPool, fragmentDescLayout[RenderPassType::FORWARD]);
		swapChainFrames[i].vertexDescSet[RenderPassType::SKY] = vkInit::allocateDescriptorSet(device, frameVertexDescPool, vertexDescLayout[RenderPassType::SKY]);

		// by the sheer power of having you everything else breaks?
		swapChainFrames[i].vertexDescSet[RenderPassType::PREPASS] = vkInit::allocateDescriptorSet(device, frameVertexDescPool, vertexDescLayout[RenderPassType::PREPASS]);
		swapChainFrames[i].fragDescSet[RenderPassType::DEFERRED] = vkInit::allocateDescriptorSet(device, frameFragmentDescPoolDeferred, fragmentDescLayout[RenderPassType::DEFERRED]);
	}
}

void Engine::finalizeSetup() {
	createFrameBuffers();

	commandPool = vkInit::makeCommandPool(device, physicalDevice, surface, debugMode);
	
	vkInit::commandBufferInput cbInput{device, commandPool, swapChainFrames };
	mainCommandBuffer = vkInit::makeCommandBuffer(cbInput, debugMode);
	makeFrameCommandBuffers(cbInput, debugMode);

	// sync creation
	createFrameResources();
}

void Engine::renderObjects(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount) {

	int indexCount = meshes->indexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	textures[objectType]->use(commandBuffer, pipelineLayouts[RenderPassType::FORWARD], 2);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}

void Engine::renderObjectsPrepass(vk::CommandBuffer commandBuffer, std::string objectType, uint32_t& startInstance, uint32_t instanceCount) {
	int indexCount = meshes->indexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	textures[objectType]->use(commandBuffer, pipelineLayouts[RenderPassType::PREPASS], 1);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}

void Engine::drawPrepass(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[RenderPassType::PREPASS];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[RenderPassType::PREPASS];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	vk::ClearValue clearDepth;
	clearDepth.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
	std::vector<vk::ClearValue> clearValues = { {clearColor, clearColor, clearColor, clearDepth} };
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();


	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[RenderPassType::PREPASS], 0, swapChainFrames[imageIndex].vertexDescSet[RenderPassType::PREPASS], nullptr);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[RenderPassType::PREPASS]);

	prepareScene(commandBuffer);

	// pass in data
	uint32_t startInstance = 0;
	for (std::pair<std::string, std::vector<talos::MeshActor>> pair : scene->gameObjects) {
		talos::MeshActor meshActor = pair.second[0];
		if (meshActor.getStaticMesh()->renderPass == "PREPASS") {
			renderObjectsPrepass(commandBuffer, pair.first, startInstance, pair.second.size());
		}
	}

	commandBuffer.endRenderPass();
}

void Engine::drawStandard(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[RenderPassType::FORWARD];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[RenderPassType::FORWARD];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	vk::ClearValue clearDepth;
	clearDepth.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
	std::vector<vk::ClearValue> clearValues = { {clearColor, clearDepth} };
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();


	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[RenderPassType::FORWARD], 0, swapChainFrames[imageIndex].vertexDescSet[RenderPassType::FORWARD], nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[RenderPassType::FORWARD], 1, swapChainFrames[imageIndex].fragDescSet[RenderPassType::FORWARD], nullptr);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[RenderPassType::FORWARD]);

	prepareScene(commandBuffer);

	// pass in data
	uint32_t startInstance = 0;
	for (std::pair<std::string, std::vector<talos::MeshActor>> pair : scene->gameObjects) {
		talos::MeshActor meshActor = pair.second[0];
		if (meshActor.getStaticMesh()->renderPass == "FORWARD") {
			renderObjectsPrepass(commandBuffer, pair.first, startInstance, pair.second.size());
		}
	}

	commandBuffer.endRenderPass();
}

void Engine::drawDeferred(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[RenderPassType::DEFERRED];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[RenderPassType::DEFERRED];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	vk::ClearValue clearDepth;
	clearDepth.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
	std::vector<vk::ClearValue> clearValues = { {clearColor, clearDepth} };
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();


	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[RenderPassType::DEFERRED], 0, swapChainFrames[imageIndex].fragDescSet[RenderPassType::DEFERRED], nullptr);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[RenderPassType::DEFERRED]);

	// pass in data- this destroys image views... isn't that just dandy?
	swapChainFrames[imageIndex].albedoTexture.use(commandBuffer, pipelineLayouts[RenderPassType::DEFERRED], 1);
	swapChainFrames[imageIndex].normalTexture.use(commandBuffer, pipelineLayouts[RenderPassType::DEFERRED], 1);
	swapChainFrames[imageIndex].prepassDepthTexture.use(commandBuffer, pipelineLayouts[RenderPassType::DEFERRED], 1);

	commandBuffer.draw(6, 1, 0, 0);

	commandBuffer.endRenderPass();
}

void Engine::drawSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[RenderPassType::SKY];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[RenderPassType::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	std::vector<vk::ClearValue> clearValues = { {clearColor} };
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();


	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[RenderPassType::SKY]);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[RenderPassType::SKY], 0, swapChainFrames[imageIndex].vertexDescSet[RenderPassType::SKY], nullptr);

	skybox->use(commandBuffer, pipelineLayouts[RenderPassType::SKY], 1);
	commandBuffer.draw(6, 1, 0, 0);
	commandBuffer.endRenderPass();

}

void Engine::render(Scene* scene) {

	if (!scene->assetsLoaded) {
		makeAssets(scene);
		scene->assetsLoaded = true;
	}

	device.waitForFences(1, &swapChainFrames[frameNumber].inFlightFence, VK_TRUE, UINT64_MAX);
	device.resetFences(1, &swapChainFrames[frameNumber].inFlightFence);
	uint32_t imageIndex;
	try {
		vk::ResultValue acquireImageResult = device.acquireNextImageKHR(swapChain, UINT64_MAX, swapChainFrames[frameNumber].renderSemaphore, nullptr);
		imageIndex = acquireImageResult.value;
	}
	catch (vk::OutOfDateKHRError) {
		recreateSwapchain();
		return;
	}

	vk::CommandBuffer commandBuffer = swapChainFrames[frameNumber].commandBuffer;

	commandBuffer.reset();

	// send in UBO data
	prepareFrame(imageIndex, scene);

	// initialize draw commands
	vk::CommandBufferBeginInfo beginInfo = {};
	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		if (debugMode) {
			std::cout << "Couldn't start the command buffer. Ending record now " << std::endl;
		}

		return;
	}

	if (scene->isPassRequired(RenderPassType::SKY)) {
		drawSky(commandBuffer, imageIndex, scene);
	}
	
	if (scene->isPassRequired(RenderPassType::PREPASS)) {
		drawPrepass(commandBuffer, imageIndex, scene);
	}

	commandBuffer.end();

	vk::SubmitInfo prepassSubmit = {};
	vk::Semaphore prepassWaitSemaphores[] = { swapChainFrames[frameNumber].renderSemaphore };
	vk::PipelineStageFlags prepassWaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eAllGraphics };
	prepassSubmit.waitSemaphoreCount = 1;
	prepassSubmit.pWaitSemaphores = prepassWaitSemaphores;
	prepassSubmit.pWaitDstStageMask = prepassWaitStages;
	prepassSubmit.pCommandBuffers = &commandBuffer;
	prepassSubmit.commandBufferCount = 1;

	graphicsQueue.submit(prepassSubmit, swapChainFrames[frameNumber].prepassFence);
	device.waitForFences(1, &swapChainFrames[frameNumber].prepassFence, VK_TRUE, UINT64_MAX);
	device.resetFences(1, &swapChainFrames[frameNumber].prepassFence);

	// commandBuffer.reset();
	// initialize draw commands
	// restart command buffer
	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		if (debugMode) {
			std::cout << "Couldn't start the command buffer. Ending record now " << std::endl;
		}

		return;
	}

	/*
		Setup textures from prepass
	*/
	vkImage::ImageLayoutTransitionInput layoutTransition;
	layoutTransition.image = swapChainFrames[imageIndex].albedoBuffer;
	layoutTransition.arrayCount = 1;
	layoutTransition.commandBuffer = mainCommandBuffer;
	layoutTransition.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	layoutTransition.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	layoutTransition.queue = graphicsQueue;
	vkImage::transitionImageLayout(layoutTransition);

	layoutTransition.image = swapChainFrames[imageIndex].normalBuffer;
	vkImage::transitionImageLayout(layoutTransition);

	layoutTransition.image = swapChainFrames[imageIndex].prepassDepthBuffer;
	layoutTransition.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	layoutTransition.aspect = vk::ImageAspectFlagBits::eDepth;
	vkImage::transitionImageLayout(layoutTransition);

	if (scene->isPassRequired(RenderPassType::FORWARD)) {
		drawStandard(commandBuffer, imageIndex, scene);
	}
	
	if (scene->isPassRequired(RenderPassType::DEFERRED)) {
		drawDeferred(commandBuffer, imageIndex, scene);
	}

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {
		if (debugMode) {
			std::cout << "Couldn't stop the command buffer. Ending record now " << std::endl;
		}

		return;
	}

	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.commandBufferCount = 1;
	vk::Semaphore signalSemaphores[] = { swapChainFrames[frameNumber].presentSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	
	try {
		graphicsQueue.submit(submitInfo, swapChainFrames[frameNumber].inFlightFence);
	}
	catch (vk::SystemError) {
		if (debugMode) {
			std::cout << "Couldn't submit to graphics queue" << std::endl;
		}
	}

	// present stage
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	vk::SwapchainKHR swapchains[] = {swapChain};
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	try {
		presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError) {
		recreateSwapchain();
		return;
	}
	
	frameNumber = (frameNumber + 1) % maxFramesInFlight;
}

void Engine::makeWorkerThreads() {
	size_t threadCount = std::thread::hardware_concurrency() - 1;

	workers.reserve(threadCount);
	vkInit::commandBufferInput commandBufferInput = { device, commandPool, swapChainFrames };
	for (size_t i = 0; i < threadCount; i++) {
		vk::CommandBuffer commandBuffer = vkInit::makeCommandBuffer(commandBufferInput);
		workers.push_back(
			std::thread(vkJob::WorkerThread(workQueue, commandBuffer, graphicsQueue))
		);
	}
}

void Engine::endWorkerThreads() {
	size_t threadCount = std::thread::hardware_concurrency() - 1;
	for (size_t i = 0; i < workers.size(); i++) {
		workers[i].join();
	}
	workers.clear();
}

// TODO: Dynamic asset loading
void Engine::makeAssets(Scene* scene) {
	meshes = new VertexCollection();

	std::unordered_map<std::string, std::vector<std::string>> modelPaths;
	std::unordered_map<std::string, std::vector<std::string>> texturePaths;

	// Load all game objects needed for the scene
	for (std::string gameObjectPath : scene->gameObjectAssetPaths) {
		std::unordered_map<std::string, std::vector<std::string>> assetPaths = talos::util::getAssetDependencies(gameObjectPath.c_str(), debugMode);
		std::vector<std::string> combinedModelMaterialPaths = assetPaths.at("model");
		std::vector<std::string> materialPaths = assetPaths.at("material");
		combinedModelMaterialPaths.insert(combinedModelMaterialPaths.end(), materialPaths.begin(), materialPaths.end());

		modelPaths.insert({ gameObjectPath, combinedModelMaterialPaths });
		texturePaths.insert({ gameObjectPath, assetPaths.at("texture") });
	}

	// Make descriptor pool
	vkInit::DescriptorSetLayoutData texLayoutData;
	texLayoutData.count = 1;
	texLayoutData.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(texturePaths.size() + 1 + 3 * swapChainFrames.size()), texLayoutData);

	// Allocate descriptors for the buffers
	for (int i = 0; i < maxFramesInFlight; i++) {
		swapChainFrames[i].createPrepassBufferTextures(meshDescPool, meshDescLayout[RenderPassType::DEFERRED]);
	}

	std::unordered_map<std::string, vkMesh::ObjMesh> loadedMeshes;
	for (std::pair<std::string, std::vector<std::string>> pair : modelPaths) {
		vkMesh::ObjMesh mesh{}; 
		loadedMeshes.emplace(pair.first, mesh);
		// w might need to be zero
		glm::mat4 preTransform = glm::mat4(1.0f);
		preTransform[3][3] = 0.0f;
		workQueue.add(new vkJob::LoadModelJob(loadedMeshes[pair.first], pair.second[0], pair.second[1], preTransform));
	}

	vkImage::TextureInput texInfo;

	texInfo.device = device;
	texInfo.physicalDevice = physicalDevice;
	texInfo.layout = meshDescLayout[RenderPassType::FORWARD];
	texInfo.pool = meshDescPool;
	texInfo.texType = vk::ImageViewType::e2D;
	texInfo.dstBinding = 0;

	/*for (const auto& [object, filename] : texturePaths) {
		texInfo.filename = filename;
		textures[object] = new vkImage::Texture{};
		workQueue.add(new vkJob::LoadTextureJob(textures[object], texInfo));
	}*/

	texInfo.layout = meshDescLayout[RenderPassType::PREPASS];
	for (const auto& [object, filename] : texturePaths) {
		texInfo.filename = filename;
		textures[object] = new vkImage::Texture{};
		workQueue.add(new vkJob::LoadTextureJob(textures[object], texInfo));
	}

	makeWorkerThreads();

	// Prepare skybox
	for (std::string skyboxPath : scene->skyboxes) {
		std::unordered_map<std::string, std::vector<std::string>> skyboxPaths = talos::util::getAssetDependencies(skyboxPath.c_str(), debugMode);
		texInfo.commandBuffer = mainCommandBuffer;
		texInfo.queue = graphicsQueue;
		texInfo.layout = meshDescLayout[RenderPassType::SKY];
		texInfo.texType = vk::ImageViewType::eCube;
		texInfo.filename = skyboxPaths.at("texture");
		skybox = new vkImage::Texture{};
		skybox->load(texInfo);
	}

	endWorkerThreads();

	for (auto meshPair : loadedMeshes) {
		meshes->consume(meshPair.first, meshPair.second.vertices, meshPair.second.indices);
	}

	FinalizationInput input;
	input.device = device;
	input.physicalDevice = physicalDevice;
	input.queue = graphicsQueue;
	input.commandBuffer = mainCommandBuffer;
	meshes->finalize(input);
}

void Engine::prepareFrame(uint32_t imageIndex, const Scene* scene) {
	vkUtilities::SwapChainFrame& frame = swapChainFrames[imageIndex];

	// TODO: Make this information part of scene
	glm::vec3 eye = { 0.0f, 0.0f, -5.0f };
	glm::vec3 center = { 0.0f, 0.0f, 0.0f };
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	glm::mat4 view = glm::lookAt(eye, center, up);

	glm::vec4 forward = glm::vec4(normalize(center - eye), 1.0);
	glm::vec4 right = glm::vec4(normalize(glm::cross(glm::vec3(forward.x, forward.y, forward.z), up)), 1.0);
	frame.cameraVectorData.forward = forward;
	frame.cameraVectorData.up = glm::vec4(up, 1.0);
	frame.cameraVectorData.right = right;
	memcpy(frame.cameraVectorWriteLocation, &(frame.cameraVectorData), sizeof(vkUtilities::CameraVectors));

	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
		0.1f,
		10.0f
	);
	projection[1][1] *= -1;
	frame.cameraMatrixData.view = view;
	frame.cameraMatrixData.projection = projection;
	frame.cameraMatrixData.viewProjection = projection * view;
	memcpy(frame.cameraMatrixWriteLocation, &(frame.cameraMatrixData), sizeof(vkUtilities::CameraMatrices));

	size_t i = 0;
	for (std::pair<std::string, std::vector<talos::MeshActor>> pair : scene->gameObjects) {
		for (talos::MeshActor& meshActor : pair.second) {
			frame.modelTransforms[i] = glm::translate(glm::mat4(1.0f), meshActor.getTransform()->position);
			i++;
		}
	}

	memcpy(frame.modelTransformWriteLocation, frame.modelTransforms.data(), scene->gameObjects.size() * sizeof(glm::mat4));

	// Create transformed lights and pass them over
	std::vector<Light> lightsInCS;
	for (Light light : scene->lights) {
		Light lightInCS{ frame.cameraMatrixData.view * glm::vec4(light.position, 1.0f), light.color };
		lightsInCS.push_back(lightInCS);
	}
	frame.updateLightInformation(lightsInCS);
	// TODO: Figure out how to force all data through
	memcpy(frame.lightWriteLocation, &(frame.lightData), sizeof(glm::vec4) * 2 * 16 + sizeof(glm::vec4));

	frame.createDescriptorSets();
	frame.writeDescriptorSets();
}

void Engine::prepareScene(vk::CommandBuffer commandBuffer) {
	vk::Buffer vertexBuffers[] = { meshes->vertexBuffer.buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(meshes->indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

Engine::~Engine() {

	device.waitIdle();

	delete meshes;
	for (const auto& [object, texture] : textures) {
		texture->destroyImage();
		texture->destroySampler();
	}

	if (skybox) {
		skybox->destroyImage();
		skybox->destroySampler();
	}

	if (window) {
		delete window;
	}

	if (debugMessenger) {
		instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	}

	if (device) {
		device.destroyCommandPool(commandPool);
		device.destroyRenderPass(renderPasses[RenderPassType::FORWARD]);
		device.destroyPipelineLayout(pipelineLayouts[RenderPassType::FORWARD]);
		device.destroyPipeline(pipelines[RenderPassType::FORWARD]);
		device.destroyRenderPass(renderPasses[RenderPassType::SKY]);
		device.destroyPipelineLayout(pipelineLayouts[RenderPassType::SKY]);
		device.destroyPipeline(pipelines[RenderPassType::SKY]);
		device.destroyRenderPass(renderPasses[RenderPassType::PREPASS]);
		device.destroyPipelineLayout(pipelineLayouts[RenderPassType::PREPASS]);
		device.destroyPipeline(pipelines[RenderPassType::PREPASS]);
		device.destroyRenderPass(renderPasses[RenderPassType::DEFERRED]);
		device.destroyPipelineLayout(pipelineLayouts[RenderPassType::DEFERRED]);
		device.destroyPipeline(pipelines[RenderPassType::DEFERRED]);
		destroySwapchain();
		device.destroyDescriptorSetLayout(vertexDescLayout[RenderPassType::FORWARD]);
		device.destroyDescriptorSetLayout(fragmentDescLayout[RenderPassType::FORWARD]);
		device.destroyDescriptorSetLayout(meshDescLayout[RenderPassType::FORWARD]);
		device.destroyDescriptorSetLayout(vertexDescLayout[RenderPassType::SKY]);
		device.destroyDescriptorSetLayout(meshDescLayout[RenderPassType::SKY]);
		device.destroyDescriptorSetLayout(vertexDescLayout[RenderPassType::PREPASS]);
		device.destroyDescriptorSetLayout(meshDescLayout[RenderPassType::PREPASS]);
		device.destroyDescriptorSetLayout(meshDescLayout[RenderPassType::DEFERRED]);
		device.destroyDescriptorSetLayout(fragmentDescLayout[RenderPassType::DEFERRED]);
		device.destroyDescriptorPool(meshDescPool);
		device.destroy();
	}

	if (instance) {
		if (surface) {
			instance.destroySurfaceKHR(surface);
		}
		instance.destroy();
	}

	
	// glfwTerminate();
}


