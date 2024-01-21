#include "ObjMesh.h"

namespace vkMesh {
	void ObjMesh::load(std::string objFilepath, std::string mtlFilepath, glm::mat4 preTransform) {
		this->preTransform = preTransform;

		std::ifstream file;
		file.open(mtlFilepath);
		std::string line;
		std::string materialName;
		std::vector<std::string> words;

		glm::vec3 ka;
		glm::vec3 kd;
		glm::vec3 ks;
		float e;
		while (std::getline(file, line)) {
			words = split(line, " ");
			
			if (!words[0].compare("newmtl")) {
				materialName = words[1];
			}

			if (!words[0].compare("Ka")) {
				ka = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
			}

			if (!words[0].compare("Kd")) {
				kd = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
			}

			if (!words[0].compare("Ks")) {
				ks = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
			}

			if (!words[0].compare("Ni")) {
				e = std::stof(words[1]);
				materials.insert({ materialName, Material{ka, kd, ks, e} });
			}
		}

		file.close();

		// OBJ File
		file.open(objFilepath);
		while (std::getline(file, line)) {
			words = split(line, " ");

			if (!words[0].compare("v")) {
				readVertexData(words);
			}

			if (!words[0].compare("vt")) {
				readTexCoordData(words);
			}

			if (!words[0].compare("vn")) {
				readNormalData(words);
			}

			if (!words[0].compare("usemtl")) {
				if (materials.find(words[1]) != materials.end()) {
					brushColor = materials[words[1]];
				}
				else {
					brushColor = Material{};
				}
			}

			if (!words[0].compare("f")) {
				readFaceData(words);
			}
		}

		file.close();

	}

	void ObjMesh::readVertexData(const std::vector<std::string>& words) {
		glm::vec4 newVertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f);
		glm::vec3 vertexPos = glm::vec3(preTransform * newVertex);
		v.push_back(vertexPos);
	}
	void ObjMesh::readTexCoordData(const std::vector<std::string>& words) {
		glm::vec2 newVertex = glm::vec2(std::stof(words[1]), std::stof(words[2]));
		vt.push_back(newVertex);
	}

	void ObjMesh::readNormalData(const std::vector<std::string>& words) {
		glm::vec4 newVertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 0.0f);
		glm::vec3 vertexNormal = glm::vec3(preTransform * newVertex);
		vn.push_back(vertexNormal);
	}

	void ObjMesh::readFaceData(const std::vector<std::string>& words) {
		size_t triangleCount = words.size() - 3;

		for (int i = 0; i < triangleCount; i++) {
			readCorner(words[1]);
			readCorner(words[2 + i]);
			readCorner(words[3 + i]);
		}
	}

	void ObjMesh::readCorner(const std::string& vertexDesc) {
		if (history.find(vertexDesc) != history.end()) {
			indices.push_back(history[vertexDesc]);
			return;
		}

		uint32_t index = static_cast<uint32_t>(history.size());
		history.insert({ vertexDesc, index });
		indices.push_back(index);

		// position
		std::vector<std::string> vertexData = split(vertexDesc, "/");
		glm::vec3 pos = v[std::stol(vertexData[0]) - 1];
		vertices.push_back(pos[0]);
		vertices.push_back(pos[1]);
		vertices.push_back(pos[2]);

		// Ka
		vertices.push_back(brushColor.ka[0]);
		vertices.push_back(brushColor.ka[1]);
		vertices.push_back(brushColor.ka[2]);

		// Kd
		vertices.push_back(brushColor.kd[0]);
		vertices.push_back(brushColor.kd[1]);
		vertices.push_back(brushColor.kd[2]);

		// Ks
		vertices.push_back(brushColor.ks[0]);
		vertices.push_back(brushColor.ks[1]);
		vertices.push_back(brushColor.ks[2]);

		// e
		vertices.push_back(brushColor.e);

		// Tex Coord
		glm::vec2 texCoord = glm::vec2(0.0f, 0.0f);
		if (vertexData.size() == 3 && vertexData[1].size() > 0) {
			texCoord = vt[std::stol(vertexData[1]) - 1];
		}
		vertices.push_back(texCoord[0]);
		vertices.push_back(1.0f - texCoord[1]);

		// Normal Coords
		glm::vec3 normal = vn[std::stol(vertexData[2]) - 1];
		vertices.push_back(normal[0]);
		vertices.push_back(normal[1]);
		vertices.push_back(normal[2]);
	}
}