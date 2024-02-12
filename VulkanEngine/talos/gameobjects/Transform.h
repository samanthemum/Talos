#pragma once
#include "Component.h"
#include "../config.h"
namespace talos {
	class Transform : public Component {
		public:
			std::string getName() const {
				return "Transform";
			}

			Transform() {
				position = glm::vec3(0.0, 0.0, 0.0);
				rotation = glm::vec3(0.0, 0.0, 0.0);
				scale = glm::vec3(1.0, 1.0, 1.0);
			}
			
			Transform& operator=(const Transform& other) {
				position = other.position;
				scale = other.scale;
				rotation = other.rotation;

				return *this;
			}

			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
	};
}