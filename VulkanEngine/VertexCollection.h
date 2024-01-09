#pragma once
#include "config.h"
#include "Memory.h"

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
		void consume(const char* type, std::vector<float> vertexData, std::vector<uint32_t> indices);
		void finalize(FinalizationInput input);
		Buffer vertexBuffer;
		Buffer indexBuffer;
		std::unordered_map<const char*, int> firstIndices;
		std::unordered_map<const char*, int> indexCounts;

	private:
		int indexOffset;
		vk::Device logicalDevice;
		std::vector<float> vertexLump;
		std::vector<uint32_t> indexLump;

};