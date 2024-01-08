#include "VertexCollection.h"

VertexCollection::VertexCollection() {
	indexOffset = 0;
}

void VertexCollection::consume(meshTypes type, std::vector<float> vertexData, std::vector<uint32_t> indices) {

	// TODO: change this to be more flexible, maybe add another data type
	int vertexCount = static_cast<int>(vertexData.size() / 8);
	int indexCount = static_cast<int>(indices.size());
	int lastIndexPosition = static_cast<int>(indexLump.size());

	firstIndices.insert(std::make_pair(type, lastIndexPosition));
	indexCounts.insert(std::make_pair(type, indexCount));

	for (float attribute : vertexData) {
		vertexLump.push_back(attribute);
	}

	for (uint32_t index : indices) {
		indexLump.push_back(index + indexOffset);
	}

	indexOffset += vertexCount;
}

void VertexCollection::finalize(FinalizationInput input) {
	this->logicalDevice = input.device;
	
	// Vertex Buffer
	BufferInput inputChunk;
	inputChunk.device = this->logicalDevice;
	inputChunk.physicalDevice = input.physicalDevice;
	inputChunk.size = sizeof(float) * vertexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	Buffer stagingBuffer = vkUtilities::createBuffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	vertexBuffer = vkUtilities::createBuffer(inputChunk);

	vkUtilities::copyBuffer(stagingBuffer, vertexBuffer, inputChunk.size, input.queue, input.commandBuffer);

	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

	// Index Buffer
	inputChunk.device = this->logicalDevice;
	inputChunk.physicalDevice = input.physicalDevice;
	inputChunk.size = sizeof(uint32_t) * indexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	stagingBuffer = vkUtilities::createBuffer(inputChunk);

	memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, indexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	indexBuffer = vkUtilities::createBuffer(inputChunk);

	vkUtilities::copyBuffer(stagingBuffer, indexBuffer, inputChunk.size, input.queue, input.commandBuffer);

	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

	vertexLump.clear();
}

VertexCollection::~VertexCollection() {
	logicalDevice.destroyBuffer(vertexBuffer.buffer);
	logicalDevice.freeMemory(vertexBuffer.bufferMemory);

	logicalDevice.destroyBuffer(indexBuffer.buffer);
	logicalDevice.freeMemory(indexBuffer.bufferMemory);
}