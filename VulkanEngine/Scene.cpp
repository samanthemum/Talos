#include "Scene.h"

Scene::Scene() {
	gameObjects.insert({ "assets/cyndaquil.txt", {glm::vec3(0.0f, 0.0f, 0.0f)} });
	lights.push_back(Light{ glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) });
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

		if (objectType == "actor") {
			std::string modelFilepath;
			std::string enteredNumber;
			std::vector<glm::vec3> positions;

			ss >> modelFilepath;

			while (!ss.eof()) {
				glm::vec3 pos;
				for (int i = 0; i < 3; i++) {
					ss >> enteredNumber;
					pos[i] = std::atof(enteredNumber.c_str());
				}

				positions.push_back(pos);
			}

			gameObjects.insert({ modelFilepath, positions });
			gameObjectAssetPaths.push_back(modelFilepath);
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
		}
		else {
			std::cout << "unrecognized object type!" << std::endl;
		}
	}
	file.close();
}