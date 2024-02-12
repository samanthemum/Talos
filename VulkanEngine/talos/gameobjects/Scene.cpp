#include "Scene.h"

Scene::Scene() {
	// Default game object
	talos::Transform* transform = new talos::Transform;
	talos::StaticMesh* staticMesh = new talos::StaticMesh;
	staticMesh->modelFile = "assets/cyndaquil.txt";
	staticMesh->renderPass = "PREPASS";
	gameObjects.insert({ "assets/cyndaquil.txt", {talos::MeshActor(transform, staticMesh)}});
	lights.push_back(Light{ glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) });
}

void Scene::insertRenderPass(RenderPassType newPass) {
	for (RenderPassType renderPass : renderPasses) {
		if (newPass == renderPass) {
			return;
		}
	}

	renderPasses.push_back(newPass);
}

bool Scene::isPassRequired(RenderPassType renderPass) {
	for (RenderPassType requiredRenderPass : renderPasses) {
		if (requiredRenderPass == renderPass) {
			return true;
		}
	}

	return false;
}

// TODO: Better cleanup
Scene::~Scene() {
	/*for (std::pair<std::string, std::vector<talos::MeshActor>> pair : gameObjects) {
		for (int i = 0; i < pair.second.size(); i++) {
			pair.second.at(i).destroyGameObject();
		}
	}*/
}

Scene::Scene(std::string filepath) {
	std::ifstream file;
	file.open(filepath);
	std::unordered_map<std::string, std::vector<std::string>> assetFilepaths;

	if (!file.is_open()) {
		std::cout << "Failed to load \"" << filepath << "\"" << std::endl;
		return;
	}

	while (!file.eof()) {
		std::string line;
		getline(file, line);

		// parse line as a string stream
		std::stringstream ss(line);

		// First argument is game object type
		// Can be either actor, light, or skybox
		std::string objectType;
		ss >> objectType;

		if (objectType == "meshActor") {
			std::string modelFilepath;
			std::string renderPass;
			std::string enteredNumber;
			std::vector<talos::MeshActor> meshActors;

			ss >> modelFilepath;

			ss >> renderPass;

			talos::StaticMesh* mesh = new talos::StaticMesh;
			mesh->modelFile = modelFilepath;
			mesh->renderPass = renderPass;

			while (!ss.eof()) {
				glm::vec3 pos;
				for (int i = 0; i < 3; i++) {
					ss >> enteredNumber;
					pos[i] = std::atof(enteredNumber.c_str());
				}

				talos::Transform* transform = new talos::Transform;
				transform->position = pos;
				talos::MeshActor meshActor = talos::MeshActor(transform, mesh);
				meshActors.push_back(meshActor);
			}

			gameObjects.insert({ modelFilepath, meshActors });
			gameObjectAssetPaths.push_back(modelFilepath);

			std::vector<RenderPassType> requiredRenderPasses = getRequiredRenderPassesFromString(renderPass);
			for (RenderPassType renderPass : requiredRenderPasses) {
				insertRenderPass(renderPass);
			}
		}
		else if (objectType == "light") {
			std::string enteredNumber;
			glm::vec3 pos;
			glm::vec3 color;

			for (int i = 0; i < 3; i++) {
				ss >> enteredNumber;
				pos[i] = std::atof(enteredNumber.c_str());
			}

			for (int i = 0; i < 3; i++) {
				ss >> enteredNumber;
				color[i] = std::atof(enteredNumber.c_str());
			}

			lights.push_back(Light{ pos, color });
		}
		else if (objectType == "skybox") {
			std::string skyboxLocation;
			ss >> skyboxLocation;
			skyboxes.push_back(skyboxLocation);
			insertRenderPass(RenderPassType::SKY);
		}
		else {
			std::cout << "unrecognized object type!" << std::endl;
		}
	}
	file.close();
}