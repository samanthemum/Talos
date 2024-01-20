#pragma once
#include "config.h"
#include "RenderStructs.h"
#include "Light.h"

class Scene {
public:
	Scene();
	Scene(std::string filepath);
	std::unordered_map<std::string, std::vector<glm::vec3>> gameObjects;
	std::vector<Light> lights;
	std::vector<std::string> skyboxes;

	// Asset loading info
	std::vector<std::string> gameObjectAssetPaths;
	
	// TODO: Evaluate moving this
	bool assetsLoaded = false;
};
