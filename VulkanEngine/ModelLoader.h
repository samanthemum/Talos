#pragma once
#include "config.h"

namespace talos {
	namespace util {
		static std::unordered_map<std::string, std::vector<std::string>> getAssetDependencies(const char* assetFilepath, bool debug = false) {
			std::ifstream file;
			file.open(assetFilepath);
			std::unordered_map<std::string, std::vector<std::string>> assetFilepaths;

			if (debug && !file.is_open()) {
				std::cout << "Failed to load \"" << assetFilepath << "\"" << std::endl;
				return assetFilepaths;
			}
			
			while (!file.eof()) {
				std::string line;
				getline(file, line);

				// parse line as a string stream
				std::stringstream ss(line);

				// First argument is asset type
				// Can be either model, material or texture
				std::string dependencyType;
				ss >> dependencyType;
				
				// Second argument is filepath to asset
				std::string dependencyFilepath;
				ss >> dependencyFilepath;

				std::vector<std::string> currentPaths = {};
				if (assetFilepaths.count(dependencyType)) {
					currentPaths = assetFilepaths.at(dependencyType);
				}
				currentPaths.push_back(dependencyFilepath);
				assetFilepaths[dependencyType] = currentPaths;
			}
			file.close();

			return assetFilepaths;
		}
	}
}