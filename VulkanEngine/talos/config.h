#pragma once
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <unordered_map>
#include <thread>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct BufferInput {
	size_t size;
	vk::BufferUsageFlags usage;
	vk::Device device;
	vk::PhysicalDevice physicalDevice;
	vk::MemoryPropertyFlags memoryProperties;
};

struct Buffer {
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;
};

enum class RenderPassType {
	SKY,
	FORWARD,
	PREPASS,
	DEFERRED
};

std::vector<RenderPassType> getRequiredRenderPassesFromString(std::string renderPassString);

std::vector<std::string> split(std::string line, std::string delimiter);