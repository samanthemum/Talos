#pragma once
#include "Component.h"
#include "../config.h"

namespace talos {
	class GameObject {
		public:
			std::vector<Component*> components;
			
			GameObject() {};

			void destroyGameObject() {
				while (!components.empty()) {
					delete components.at(0);
				}
			}

			void addComponent(Component* component) {
				components.push_back(component);
			}

			std::vector<Component*> getComponents(const std::string componentName) const {
				std::vector<Component*> componentsWithType;
				for (int i = 0; i < components.size(); i++) {
					if (components.at(i)->getName() == componentName) {
						componentsWithType.push_back(components.at(i));
					}
				}

				return componentsWithType;
			}
	};
}