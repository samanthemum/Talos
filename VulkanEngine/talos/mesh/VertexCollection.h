#pragma once
#include "../config.h"
#include "../utilities/Memory.h"

struct FinalizationInput {
	vk::Device device;
		vk::PhysicalDevice physicalDevice;
		vk::Queue queue;
		vk::CommandBuffer commandBuffer;
};

class VertexCollection {
	public:
		VertexCollection();
		~VertexCollection();
		void consume(std::string type, std::vector<float> vertexData, std::vector<uint32_t> indices);
		void finalize(FinalizationInput input);
		Buffer vertexBuffer;
		Buffer indexBuffer;
		std::unordered_map<std::string, int> firstIndices;
		std::unordered_map<std::string, int> indexCounts;

	private:
		int indexOffset;
		vk::Device logicalDevice;
		std::vector<float> vertexLump;
		std::vector<uint32_t> indexLump;

};