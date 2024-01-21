#include "Engine.h"

#include <iostream>
#include "init/Instance.h"
#include "utilities/Logging.h"
#include "init/Device.h"
#include "init/Swapchain.h"
#include "init/Pipeline.h"
#include "init/Framebuffer.h"
#include "init/Command.h"
#include "init/Synchronization.h"
#include "init/Descriptors.h"
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
	}
}

void Engine::destroySwapchain() {
	for (vkUtilities::SwapChainFrame& frame : swapChainFrames) {
		frame.destroy();
	}

	device.destroySwapchainKHR(swapChain);

	device.destroyDescriptorPool(frameVertexDescPool);
	device.destroyDescriptorPool(frameFragmentDescPool);
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
	vertexDescLayout[PipelineTypes::SKY] = vkInit::makeDescriptorSetLayout(device, bindings);

	bindings.count = 2;
	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.shaderStages.push_back(vk::ShaderStageFlagBits::eVertex);
	vertexDescLayout[PipelineTypes::FORWARD] = vkInit::makeDescriptorSetLayout(device, bindings);
	

	// Fragment shader descriptor bindings
	vkInit::DescriptorSetLayoutData fragmentBindings;
	fragmentBindings.count = 1;

	fragmentBindings.indices.push_back(0);
	fragmentBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	fragmentBindings.counts.push_back(1);
	fragmentBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);

	fragmentDescLayout[PipelineTypes::FORWARD] = vkInit::makeDescriptorSetLayout(device, fragmentBindings);

	vkInit::DescriptorSetLayoutData meshBindings;
	meshBindings.count = 1;
	meshBindings.indices.push_back(0);
	meshBindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshBindings.counts.push_back(1);
	meshBindings.shaderStages.push_back(vk::ShaderStageFlagBits::eFragment);

	// TODO: Figure out a better way to do this
	meshDescLayout[PipelineTypes::FORWARD] = vkInit::makeDescriptorSetLayout(device, meshBindings);
	meshDescLayout[PipelineTypes::SKY] = vkInit::makeDescriptorSetLayout(device, meshBindings);
}

void Engine::setupPipeline() {
	vkInit::PipelineBuilder pipelineBuilder(device);

	// Sky
	pipelineBuilder.setColorOverwrite(true);
	pipelineBuilder.specifyVertexShader("Shaders/sky_vert.spv");
	pipelineBuilder.specifyFragmentShader("Shaders/sky_frag.spv");
	pipelineBuilder.specifySwapchainExtent(swapChainExtent);
	pipelineBuilder.clearDepthAttachment();
	pipelineBuilder.addDescriptorSetLayout(vertexDescLayout[PipelineTypes::SKY]);
	pipelineBuilder.addDescriptorSetLayout(meshDescLayout[PipelineTypes::SKY]);
	pipelineBuilder.addColorAttachment(swapChainFormat, 0);

	pipelineBuilder.setDepthTest(false);

	vkInit::GraphicsPipelineOutBundle output = pipelineBuilder.build();

	// Should be sky?
	pipelineLayouts[PipelineTypes::SKY] = output.layout;
	renderPasses[PipelineTypes::SKY] = output.renderPass;
	pipelines[PipelineTypes::SKY] = output.pipeline;

	pipelineBuilder.reset();

	// Forward
	pipelineBuilder.setColorOverwrite(false);
	pipelineBuilder.specifyVertexFormat(vkMesh::getPosColorBindingDescription(), vkMesh::getPosColorAttributeDescriptions());
	pipelineBuilder.specifyVertexShader("Shaders/vert.spv");
	pipelineBuilder.specifyFragmentShader("Shaders/frag.spv");
	pipelineBuilder.specifySwapchainExtent(swapChainExtent);
	pipelineBuilder.specifyDepthAttachment(swapChainFrames[0].depthBufferFormat, 1);
	pipelineBuilder.addDescriptorSetLayout(vertexDescLayout[PipelineTypes::FORWARD]);
	pipelineBuilder.addDescriptorSetLayout(fragmentDescLayout[PipelineTypes::FORWARD]);
	pipelineBuilder.addDescriptorSetLayout(meshDescLayout[PipelineTypes::FORWARD]);
	pipelineBuilder.addColorAttachment(swapChainFormat, 0);

	pipelineBuilder.setDepthTest(true);

	output = pipelineBuilder.build();

	pipelineLayouts[PipelineTypes::FORWARD] = output.layout;
	renderPasses[PipelineTypes::FORWARD] = output.renderPass;
	pipelines[PipelineTypes::FORWARD] = output.pipeline;
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
	vkInit::DescriptorSetLayoutData vertexBindings;
	vertexBindings.count = 2;
	vertexBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	vertexBindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	frameVertexDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size() * 2), vertexBindings);

	vkInit::DescriptorSetLayoutData fragmentBindings;
	fragmentBindings.count = 1;
	fragmentBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	frameFragmentDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(swapChainFrames.size() * 2), fragmentBindings);

	for (int i = 0; i < maxFramesInFlight; i++) {
		swapChainFrames[i].renderSemaphore = vkInit::makeSemaphore(device, debugMode);
		swapChainFrames[i].presentSemaphore = vkInit::makeSemaphore(device, debugMode);
		swapChainFrames[i].inFlightFence = vkInit::makeFence(device, debugMode);
		
		swapChainFrames[i].createDescriptorResources();

		swapChainFrames[i].vertexDescSet[PipelineTypes::FORWARD] = vkInit::allocateDescriptorSet(device, frameVertexDescPool, vertexDescLayout[PipelineTypes::FORWARD]);
		swapChainFrames[i].fragDescSet[PipelineTypes::FORWARD] = vkInit::allocateDescriptorSet(device, frameFragmentDescPool, fragmentDescLayout[PipelineTypes::FORWARD]);
		swapChainFrames[i].vertexDescSet[PipelineTypes::SKY] = vkInit::allocateDescriptorSet(device, frameVertexDescPool, vertexDescLayout[PipelineTypes::SKY]);
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
	textures[objectType]->use(commandBuffer, pipelineLayouts[PipelineTypes::FORWARD], 2);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}

void Engine::drawStandard(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[PipelineTypes::FORWARD];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[PipelineTypes::FORWARD];
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
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[PipelineTypes::FORWARD], 0, swapChainFrames[imageIndex].vertexDescSet[PipelineTypes::FORWARD], nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[PipelineTypes::FORWARD], 1, swapChainFrames[imageIndex].fragDescSet[PipelineTypes::FORWARD], nullptr);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[PipelineTypes::FORWARD]);

	prepareScene(commandBuffer);

	// pass in data
	uint32_t startInstance = 0;
	for (std::pair<std::string, std::vector<glm::vec3>> pair : scene->gameObjects) {
		renderObjects(commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size()));
	}

	commandBuffer.endRenderPass();
}

void Engine::drawSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPasses[PipelineTypes::SKY];
	renderPassInfo.framebuffer = swapChainFrames[imageIndex].frameBuffer[PipelineTypes::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	std::vector<vk::ClearValue> clearValues = { {clearColor} };
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();


	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[PipelineTypes::SKY]);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts[PipelineTypes::SKY], 0, swapChainFrames[imageIndex].vertexDescSet[PipelineTypes::SKY], nullptr);

	skybox->use(commandBuffer, pipelineLayouts[PipelineTypes::SKY], 1);
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

	drawSky(commandBuffer, imageIndex, scene);

	drawStandard(commandBuffer, imageIndex, scene);

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
	vk::Semaphore waitSemaphores[] = { swapChainFrames[frameNumber].renderSemaphore };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
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
	meshDescPool = vkInit::createDescriptorPool(device, static_cast<uint32_t>(texturePaths.size() + 1), texLayoutData);

	std::unordered_map<std::string, vkMesh::ObjMesh> loadedMeshes;
	for (std::pair<std::string, std::vector<std::string>> pair : modelPaths) {
		vkMesh::ObjMesh mesh{}; 
		loadedMeshes.emplace(pair.first, mesh);
		workQueue.add(new vkJob::LoadModelJob(loadedMeshes[pair.first], pair.second[0], pair.second[1], glm::mat4(1.0f)));
	}

	vkImage::TextureInput texInfo;

	texInfo.device = device;
	texInfo.physicalDevice = physicalDevice;
	texInfo.layout = meshDescLayout[PipelineTypes::FORWARD];
	texInfo.pool = meshDescPool;
	texInfo.texType = vk::ImageViewType::e2D;

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
		texInfo.layout = meshDescLayout[PipelineTypes::SKY];
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
	for (std::pair<std::string, std::vector<glm::vec3>> pair : scene->gameObjects) {
		for (glm::vec3& position : pair.second) {
			frame.modelTransforms[i] = glm::translate(glm::mat4(1.0f), position);
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
		delete texture;
	}

	if (skybox) {
		delete skybox;
	}

	if (window) {
		delete window;
	}

	if (debugMessenger) {
		instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	}

	if (device) {
		device.destroyCommandPool(commandPool);
		device.destroyRenderPass(renderPasses[PipelineTypes::FORWARD]);
		device.destroyPipelineLayout(pipelineLayouts[PipelineTypes::FORWARD]);
		device.destroyPipeline(pipelines[PipelineTypes::FORWARD]);
		device.destroyRenderPass(renderPasses[PipelineTypes::SKY]);
		device.destroyPipelineLayout(pipelineLayouts[PipelineTypes::SKY]);
		device.destroyPipeline(pipelines[PipelineTypes::SKY]);
		destroySwapchain();
		device.destroyDescriptorSetLayout(vertexDescLayout[PipelineTypes::FORWARD]);
		device.destroyDescriptorSetLayout(fragmentDescLayout[PipelineTypes::FORWARD]);
		device.destroyDescriptorSetLayout(meshDescLayout[PipelineTypes::FORWARD]);
		device.destroyDescriptorSetLayout(vertexDescLayout[PipelineTypes::SKY]);
		device.destroyDescriptorSetLayout(meshDescLayout[PipelineTypes::SKY]);
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


