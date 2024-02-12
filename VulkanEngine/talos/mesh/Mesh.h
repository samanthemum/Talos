#pragma once
#include "../config.h"

// Refactor so stride doesn't have to be hardcoded ffs
namespace vkMesh {
	static int NUM_ATTRIBUTES = 7;

	// Outputs vertex input for position and color
	inline vk::VertexInputBindingDescription getPosColorBindingDescription() {

		vk::VertexInputBindingDescription bindingDesc;
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(float) * 18;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex;

		return bindingDesc;
	}

	inline std::vector<vk::VertexInputAttributeDescription> getPosColorAttributeDescriptions() {
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::VertexInputAttributeDescription temp;

		for (int i = 0; i < NUM_ATTRIBUTES; i++) {
			attributes.push_back(temp);
		}

		// position
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = vk::Format::eR32G32B32Sfloat;
		attributes[0].offset = 0;

		// Ka
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = vk::Format::eR32G32B32Sfloat;
		attributes[1].offset = 3 * sizeof(float);

		// Kd
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = vk::Format::eR32G32B32Sfloat;
		attributes[2].offset = 6 * sizeof(float);

		// Ks
		attributes[3].binding = 0;
		attributes[3].location = 3;
		attributes[3].format = vk::Format::eR32G32B32Sfloat;
		attributes[3].offset = 9 * sizeof(float);

		// e
		attributes[4].binding = 0;
		attributes[4].location = 4;
		attributes[4].format = vk::Format::eR32Sfloat;
		attributes[4].offset = 12 * sizeof(float);

		// UV Coordinates
		attributes[5].binding = 0;
		attributes[5].location = 5;
		attributes[5].format = vk::Format::eR32G32Sfloat;
		attributes[5].offset = 13 * sizeof(float);

		// Normal Coordinates
		attributes[6].binding = 0;
		attributes[6].location = 6;
		attributes[6].format = vk::Format::eR32G32B32Sfloat;
		attributes[6].offset = 15 * sizeof(float);;

		return attributes;
	}
};