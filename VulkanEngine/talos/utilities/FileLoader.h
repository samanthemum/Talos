#pragma once
#include "../config.h"

namespace vkUtilities {
	inline std::vector<char> readFile(std::string filename, bool debug = false) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (debug && !file.is_open()) {
			std::cout << "Failed to load \"" << filename << "\"" << std::endl;
		}

		// get number of bytes/characters
		size_t filesize(static_cast<size_t>(file.tellg()));

		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
	}

	inline vk::ShaderModule createModule(std::string filename, vk::Device device, bool debug = false) {
		std::vector<char> fileContents = readFile(filename, debug);

		// setup create info
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.flags = vk::ShaderModuleCreateFlags();
		createInfo.codeSize = fileContents.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContents.data());

		try {
			return device.createShaderModule(createInfo);
		}
		catch (vk::SystemError) {
			if (debug) {
				std::cout << "Shader module creation encountered errors" << std::endl;
			}

			return nullptr;
		}
	}
}