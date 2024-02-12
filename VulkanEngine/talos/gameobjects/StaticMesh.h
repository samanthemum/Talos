#pragma once
#include "Component.h"
#include "../config.h"
namespace talos {
	class StaticMesh : public Component {
	public:
		std::string getName() const {
			return "StaticMesh";
		}

		StaticMesh() {}

		StaticMesh& operator=(const StaticMesh& other) {
			modelFile = other.modelFile;
			renderPass = other.renderPass;

			return *this;
		}

		std::string modelFile = "";
		std::string renderPass = "";
	};
}