#include "Scene.h"

Scene::Scene() {
	positions.insert({ "cyndaquil", {glm::vec3(0.0f, 0.0f, 0.0f)} });
	lights.push_back(Light{ glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) });
}

// TODO: load objects into the scene and insert them at a certain location