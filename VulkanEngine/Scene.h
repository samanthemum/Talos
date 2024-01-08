#pragma once
#include "config.h"
#include "RenderStructs.h"
#include "Light.h"

class Scene {
public:
	Scene();
	std::unordered_map<meshTypes, std::vector<glm::vec3>> positions;
	std::vector<Light> lights;
};
