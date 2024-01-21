#pragma once
#include "../config.h"
#include "../utilities/FileLoader.h"
#include "../utilities/RenderStructs.h"
#include "../mesh/Mesh.h"

namespace vkInit {

	struct GraphicsPipelineInBundle {
		vk::Device device;
		std::string vertexShaderFilepath;
		std::string fragmentShaderFilepath;
		vk::Extent2D swapChainExtent;
		vk::Format imageFormat, depthFormat;
		std::vector<vk::DescriptorSetLayout> descLayouts;
	};

	struct GraphicsPipelineOutBundle {
		vk::PipelineLayout layout;
		vk::RenderPass renderPass;
		vk::Pipeline pipeline;
	};

	class PipelineBuilder {
		public:
			PipelineBuilder(vk::Device device);
			~PipelineBuilder();

			void reset();

			/*
				Configuration for vertex input
			*/

			void specifyVertexFormat(vk::VertexInputBindingDescription bindingDescription, std::vector<vk::VertexInputAttributeDescription> attributeDescriptions);
			void specifyVertexShader(const char* filename);

			/*
				Configuration for the fragment shader
			*/
			void specifyFragmentShader(const char* filename);

			void specifySwapchainExtent(vk::Extent2D screenSize);

			void specifyDepthAttachment(const vk::Format& depthFormat, uint32_t attachmentIndex);

			void clearDepthAttachment();

			void addColorAttachment(const vk::Format& format, uint32_t attachmentIndex);

			GraphicsPipelineOutBundle build();

			void addDescriptorSetLayout(vk::DescriptorSetLayout descriptorSetLayout);

			void resetDescriptorSetLayouts();

			void setDepthTest(bool depthTest) { shouldDepthTest = depthTest; }

			void setColorOverwrite(bool overwrite) { shouldClearColorBuffer = overwrite; }

			static vk::PipelineLayout makePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> descLayouts, bool debug = false) {

				vk::PipelineLayoutCreateInfo createInfo = {};
				createInfo.flags = vk::PipelineLayoutCreateFlags();

				createInfo.setLayoutCount = static_cast<uint32_t>(descLayouts.size());
				createInfo.pSetLayouts = descLayouts.data();

				createInfo.pushConstantRangeCount = 0;

				try {
					return device.createPipelineLayout(createInfo);
				}
				catch (vk::SystemError err) {
					if (debug) {
						std::cout << "Failed to create pipeline layout" << std::endl;
					}

					return nullptr;
				}
			}

		private:
			vk::Device device;
			vk::GraphicsPipelineCreateInfo pipelineInfo = {};

			vk::VertexInputBindingDescription bindingDescription;
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
			vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};

			std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
			vk::ShaderModule vertexShader = nullptr, fragmentShader = nullptr;
			vk::PipelineShaderStageCreateInfo vertexShaderInfo, fragmentShaderInfo;

			vk::Extent2D swapchainExtent;
			vk::Viewport viewport = {};
			vk::Rect2D scissor = {};
			vk::PipelineViewportStateCreateInfo viewportState = {};

			vk::PipelineRasterizationStateCreateInfo rasterizer = {};

			vk::PipelineDepthStencilStateCreateInfo depthState;
			std::unordered_map<uint32_t, vk::AttachmentDescription> attachmentDescriptions;
			std::unordered_map<uint32_t, vk::AttachmentReference> attachmentReferences;
			std::vector<vk::AttachmentDescription> flattenedAttachmentDescriptions;
			std::vector<vk::AttachmentReference> flattenedAttachmentReferences;

			vk::PipelineMultisampleStateCreateInfo multisampling = {};

			vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
			vk::PipelineColorBlendStateCreateInfo colorBlending = {};

			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

			bool shouldDepthTest = true;
			bool shouldClearColorBuffer = true;

			

			void resetVertexFormat();

			void resetShaderModules();

			void resetRenderpassAttachments();

			/**
				Make an attachment description

				\param format the image format for the underlying resource
				\param finalLayout the expected final layout after implicit transition (acquisition)
				\returns a description of the corresponding attachment
			*/
			vk::AttachmentDescription makeRenderpassAttachment(
				const vk::Format& format, 
				vk::AttachmentLoadOp loadOp,
				vk::AttachmentStoreOp storeOp,
				vk::ImageLayout initialLayout,
				vk::ImageLayout finalLayout);

			/**
				\returns Make a renderpass attachment reference
			*/
			vk::AttachmentReference makeAttachmentReference(
				uint32_t attachment_index, vk::ImageLayout layout);

			/**
				set up the input assembly stage
			*/
			void configureInputAssembly();

			/**
				Configure a programmable shader stage.

				\param shaderModule the compiled shader module
				\param stage the shader stage which the module is for
				\returns the shader stage creation info
			*/
			vk::PipelineShaderStageCreateInfo makeShaderInfo(
				const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage);

			/**
				Configure the pipeline's viewport stage.

				\param viewport the viewport specification
				\param scissor the scissor rectangle to apply
				\returns the viewport state creation info
			*/
			vk::PipelineViewportStateCreateInfo makeViewportState();

			/**
				sets the creation info for the configured rasterizer stage
			*/
			void makeRasterizerInfo();

			/**
				configures the multisampling stage
			*/
			void configureMultisampling();

			/**
				configures the color blending stage
			*/
			void configureColorBlending();

			/**
				Make a pipeline layout, this consists mostly of describing the
				push constants and descriptor set layouts which will be used.

				\returns the created pipeline layout
			*/
			vk::PipelineLayout makePipelineLayout();

			/**
				Make a renderpass, a renderpass describes the subpasses involved
				as well as the attachments which will be used.

				\returns the created renderpass
			*/
			vk::RenderPass makeRenderpass();

			/**
				Make a simple subpass.

				\param colorAttachmentRef a reference to a color attachment for the color buffer
				\returns a description of the subpass
			*/
			vk::SubpassDescription makeSubpass(
				const std::vector<vk::AttachmentReference>& attachments
			);

			/**
				Make a simple renderpass.

				\param colorAttachment the color attachment for the color buffer
				\param subpass a description of the subpass
				\returns creation info for the renderpass
			*/
			vk::RenderPassCreateInfo makeRenderpassInfo(
				const std::vector<vk::AttachmentDescription>& attachments,
				const vk::SubpassDescription& subpass
			);
	};
}