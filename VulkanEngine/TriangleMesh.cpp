#include "TriangleMesh.h"

TriangleMesh::TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice) {
	this->device = logicalDevice;
	std::vector<float> vertices = {
	{
			0.0f, -0.05f, 0.0f, 1.0f, 0.0f,
			0.05f, 0.05f, 0.0f, 1.0f, 0.0f,
			-0.05f, 0.05f, 0.0f, 1.0f, 0.0f
	}
	};

	BufferInput inputChunk;
	inputChunk.device = this->device;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.size = sizeof(float) * vertices.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer;

	vertexBuffer = vkUtilities::createBuffer(inputChunk);

	void* memoryLocation = device.mapMemory(vertexBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertices.data(), inputChunk.size);
	device.unmapMemory(vertexBuffer.bufferMemory);
}

TriangleMesh::~TriangleMesh() {
	device.destroyBuffer(vertexBuffer.buffer);
	device.freeMemory(vertexBuffer.bufferMemory);
}