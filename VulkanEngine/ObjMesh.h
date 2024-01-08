#pragma once
#include "config.h"

namespace vkMesh {
	struct Material {
		glm::vec3 ka;
		glm::vec3 kd;
		glm::vec3 ks;
		float e;
	};

	class ObjMesh {
	public:
		std::vector<float> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<std::string, uint32_t> history;
		std::unordered_map<std::string, Material> materials;
		Material brushColor;

		std::vector<glm::vec3> v, vn;
		std::vector<glm::vec2> vt;
		glm::mat4 preTransform;

		void load(const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform);
		void readVertexData(const std::vector<std::string>& words);
		void readTexCoordData(const std::vector<std::string>& words);
		void readNormalData(const std::vector<std::string>& words);
		void readFaceData(const std::vector<std::string>& words);
		void readCorner(const std::string& vertexDesc);
	};
}