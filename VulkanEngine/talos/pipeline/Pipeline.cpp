#include "Pipeline.h"

namespace vkInit {
	PipelineBuilder::PipelineBuilder(vk::Device device) {
		this->device = device;
		reset();

		// Set stages that have reasonable defaults
		// TODO: these should also be configurable eventually
		configureInputAssembly();
		makeRasterizerInfo();
		configureMultisampling();
		pipelineInfo.basePipelineHandle = nullptr;
	}

	PipelineBuilder::~PipelineBuilder() {
		reset();
	}

	void PipelineBuilder::reset() {
		pipelineInfo.flags = vk::PipelineCreateFlags();

		resetVertexFormat();
		resetShaderModules();
		resetRenderpassAttachments();
		resetDescriptorSetLayouts();
	}

	void PipelineBuilder::resetVertexFormat() {
		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}

	void PipelineBuilder::resetRenderpassAttachments() {
		attachmentDescriptions.clear();
		attachmentReferences.clear();
	}

	void PipelineBuilder::specifyVertexFormat(vk::VertexInputBindingDescription bindingDescription,
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions)  {

		this->bindingDescription = bindingDescription;
		this->attributeDescriptions = attributeDescriptions;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &(this->bindingDescription);
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = this->attributeDescriptions.data();
	}

	void PipelineBuilder::resetShaderModules() {
		// TODO: why separate? scary
		if (vertexShader) {
			device.destroyShaderModule(vertexShader);
			vertexShader = nullptr;
		}
		if (fragmentShader) {
			device.destroyShaderModule(fragmentShader);
			fragmentShader = nullptr;
		}

		shaderStages.clear();
	}

	void PipelineBuilder::specifyVertexShader(const char* filename) {

		if (vertexShader) {
			device.destroyShaderModule(vertexShader);
			vertexShader = nullptr;
		}

		vertexShader = vkUtilities::createModule(filename, device);
		vertexShaderInfo = makeShaderInfo(vertexShader, vk::ShaderStageFlagBits::eVertex);
		shaderStages.push_back(vertexShaderInfo);
	}

	void PipelineBuilder::specifyFragmentShader(const char* filename) {

		if (fragmentShader) {
			device.destroyShaderModule(fragmentShader);
			fragmentShader = nullptr;
		}

		fragmentShader = vkUtilities::createModule(filename, device);
		fragmentShaderInfo = makeShaderInfo(fragmentShader, vk::ShaderStageFlagBits::eFragment);
		shaderStages.push_back(fragmentShaderInfo);
	}

	vk::PipelineShaderStageCreateInfo PipelineBuilder::makeShaderInfo (
		const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage) {

		vk::PipelineShaderStageCreateInfo shaderInfo = {};
		shaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		shaderInfo.stage = stage;
		shaderInfo.module = shaderModule;
		shaderInfo.pName = "main";
		return shaderInfo;
	}

	void PipelineBuilder::specifySwapchainExtent(vk::Extent2D screen_size) {
		swapchainExtent = screen_size;
	}

	void PipelineBuilder::specifyDepthAttachment(const vk::Format& depthFormat, uint32_t attachment_index) {

		depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = true;
		depthState.depthWriteEnable = true;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = false;

		pipelineInfo.pDepthStencilState = &depthState;
		attachmentDescriptions.insert(
			{ attachment_index,
			makeRenderpassAttachment(depthFormat, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal)
			}
		);
		attachmentReferences.insert(
			{ attachment_index,
			makeAttachmentReference(attachment_index, vk::ImageLayout::eDepthStencilAttachmentOptimal)
			}
		);
	}

	void PipelineBuilder::addColorAttachment(const vk::Format& format, uint32_t attachment_index, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) {
		vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
		if (initialLayout == vk::ImageLayout::ePresentSrcKHR) {
			loadOp = vk::AttachmentLoadOp::eLoad;
		}
		vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

		attachmentDescriptions.insert(
			{ attachment_index,
			makeRenderpassAttachment(format, loadOp, storeOp, initialLayout, finalLayout)
			}
		);
		attachmentReferences.insert(
			{ attachment_index,
			makeAttachmentReference(attachment_index, vk::ImageLayout::eColorAttachmentOptimal)
			}
		);
	}

	// TODO: Make this an argument
	vk::AttachmentDescription PipelineBuilder::makeRenderpassAttachment(
		const vk::Format& format,
		vk::AttachmentLoadOp loadOp,
		vk::AttachmentStoreOp storeOp,
		vk::ImageLayout initialLayout,
		vk::ImageLayout finalLayout) {

		vk::AttachmentDescription attachment = {};
		attachment.flags = vk::AttachmentDescriptionFlags();
		attachment.format = format;
		attachment.samples = vk::SampleCountFlagBits::e1;
		attachment.loadOp = loadOp;
		attachment.storeOp = storeOp;
		attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachment.initialLayout = initialLayout;
		attachment.finalLayout = finalLayout;

		return attachment;
	}

	vk::AttachmentReference PipelineBuilder::makeAttachmentReference(
		uint32_t attachment_index, vk::ImageLayout layout) {

		vk::AttachmentReference attachmentRef = {};
		attachmentRef.attachment = attachment_index;
		attachmentRef.layout = layout;

		return attachmentRef;
	}

	void PipelineBuilder::clearDepthAttachment() {
		pipelineInfo.pDepthStencilState = nullptr;
	}

	void PipelineBuilder::addDescriptorSetLayout(vk::DescriptorSetLayout descriptorSetLayout) {
		descriptorSetLayouts.push_back(descriptorSetLayout);
	}

	void PipelineBuilder::resetDescriptorSetLayouts() {
		descriptorSetLayouts.clear();
	}

	vkInit::GraphicsPipelineOutBundle PipelineBuilder::build() {

		//Vertex Input
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		//Viewport and Scissor
		makeViewportState();
		pipelineInfo.pViewportState = &viewportState;

		//Rasterizer
		pipelineInfo.pRasterizationState = &rasterizer;

		//Shader Modules
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		//Depth-Stencil is handled by depth attachment functions.

		//Multisampling
		pipelineInfo.pMultisampleState = &multisampling;

		//Color Blend
		configureColorBlending();
		pipelineInfo.pColorBlendState = &colorBlending;

		//Pipeline Layout
		vk::PipelineLayout pipelineLayout = makePipelineLayout();
		pipelineInfo.layout = pipelineLayout;

		//Renderpass
		vk::RenderPass renderpass = makeRenderpass();
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;

		//Make the Pipeline
		vk::Pipeline graphicsPipeline;
		try {
			graphicsPipeline = (device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
		}
		catch (vk::SystemError err) {
			std::cerr << "Encountered an error while trying to create pipeline: " << err.what() << std::endl;
		}

		GraphicsPipelineOutBundle output;
		output.layout = pipelineLayout;
		output.renderPass = renderpass;
		output.pipeline = graphicsPipeline;

		return output;
	}

	void PipelineBuilder::configureInputAssembly() {

		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

	}

	vk::PipelineViewportStateCreateInfo PipelineBuilder::makeViewportState() {

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchainExtent.width;
		viewport.height = (float)swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = swapchainExtent;

		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		return viewportState;
	}

	void PipelineBuilder::makeRasterizerInfo() {

		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
	}

	void PipelineBuilder::configureMultisampling() {

		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	}

	// TODO: Realistically, this also should be hella configurable
	void PipelineBuilder::configureColorBlending() {

		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;

		colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = shouldDepthTest ? attachmentDescriptions.size() - 1 : attachmentDescriptions.size();
		for (int i = 0; i < colorBlending.attachmentCount; i++) {
			colorBlendAttachments.push_back(colorBlendAttachment);
		}
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

	}

	vk::PipelineLayout PipelineBuilder::makePipelineLayout() {

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();

		layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		layoutInfo.pSetLayouts = descriptorSetLayouts.data();

		layoutInfo.pushConstantRangeCount = 0;

		try {
			return device.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err) {
			std::cerr << "Failed to make pipeline layout due to " << err.what() << std::endl;
			return nullptr;
		}
	}

	vk::RenderPass PipelineBuilder::makeRenderpass() {

		flattenedAttachmentDescriptions.clear();
		flattenedAttachmentReferences.clear();
		size_t attachmentCount = attachmentDescriptions.size();
		flattenedAttachmentDescriptions.resize(attachmentCount);
		flattenedAttachmentReferences.resize(attachmentCount);

		for (int i = 0; i < attachmentCount; ++i) {
			flattenedAttachmentDescriptions[i] = attachmentDescriptions[i];
			flattenedAttachmentReferences[i] = attachmentReferences[i];
		}

		//Renderpasses are broken down into subpasses, there's always at least one.
		vk::SubpassDescription subpass = makeSubpass(flattenedAttachmentReferences);

		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = makeRenderpassInfo(flattenedAttachmentDescriptions, subpass);
		try {
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err) {
			std::cerr << "Failed to make render pass due to " << err.what() << std::endl;
			return nullptr;
		}

	}

	vk::SubpassDescription vkInit::PipelineBuilder::makeSubpass(
		const std::vector<vk::AttachmentReference>& attachments
		) {

		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		int numberOfColorAttachments = shouldDepthTest ? attachments.size() - 1 : attachments.size();
		subpass.colorAttachmentCount = numberOfColorAttachments;
		subpass.pColorAttachments = attachments.data();
		if (attachments[0].layout == vk::ImageLayout::ePresentSrcKHR) {
			std::cout << "Present" << std::endl;
		}

		// Always store the depth attachment at the end
		if (shouldDepthTest) {
			subpass.pDepthStencilAttachment = &attachments[attachments.size() - 1];
		}
		

		return subpass;
	}

	vk::RenderPassCreateInfo vkInit::PipelineBuilder::makeRenderpassInfo(
		const std::vector<vk::AttachmentDescription>& attachments,
		const vk::SubpassDescription& subpass) {

		vk::RenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		return renderpassInfo;
	}
}