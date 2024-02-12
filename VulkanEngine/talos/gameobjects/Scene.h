#pragma once
#include "../config.h"
#include "../utilities/RenderStructs.h"
#include "Light.h"
#include "MeshActor.h"

class Scene {
public:
	Scene();
	Scene(std::string filepath);
	~Scene();

	// For now, just mesh objects
	std::unordered_map<std::string, std::vector<talos::MeshActor>> gameObjects;
	std::vector<Light> lights;
	std::vector<std::string> skyboxes;

	// Asset loading info
	std::vector<std::string> gameObjectAssetPaths;

	// Render Pass Information
	std::vector<RenderPassType> renderPasses;
	void insertRenderPass(RenderPassType);
	bool isPassRequired(RenderPassType renderPass);
	
	// TODO: Evaluate moving this
	bool assetsLoaded = false;
};
