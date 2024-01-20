#pragma once
#include "config.h"
#include "ObjMesh.h"
#include "Image.h"
#include "Texture.h"
#include <queue>

namespace vkJob {
	enum class JobStatus {
		PENDING, 
		IN_PROGRESS,
		FINISHED
	};

	class Job {
	public:
		JobStatus status = JobStatus::PENDING;
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) = 0;
	};

	class LoadModelJob : public Job {
	public:
		std::string objFilepath;
		std::string mtlFilepath;
		glm::mat4 preTransform;
		vkMesh::ObjMesh& mesh;
		LoadModelJob(vkMesh::ObjMesh& objMesh, std::string objFilepath, std::string mtlFilepath, glm::mat4 preTransform);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class LoadTextureJob : public Job {
	public:
		vkImage::TextureInput texInfo;
		vkImage::Texture* texture;
		LoadTextureJob(vkImage::Texture* texture, vkImage::TextureInput texInfo);
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) final;
	};

	class JobQueue {
		private:
			std::queue<Job*> jobQueue;
			std::mutex lock;
			bool finished = false;
		public:
			void add(Job* job);
			Job* getNextJob();
			void clearQueue();
	};
}