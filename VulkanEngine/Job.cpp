#include "Job.h"

namespace vkJob {
	LoadModelJob::LoadModelJob(vkMesh::ObjMesh& mesh, std::string objFilepath, std::string mtlFilepath, glm::mat4 preTransform) : mesh(mesh) {
		this->objFilepath = objFilepath;
		this->mtlFilepath = mtlFilepath;
		this->preTransform = preTransform;
	}

	// TODO: Removed unused parameters
	void LoadModelJob::execute(vk::CommandBuffer commandBuffer, vk::Queue queue) {
		mesh.load(objFilepath, mtlFilepath, preTransform);
		status = JobStatus::FINISHED;
	}

	LoadTextureJob::LoadTextureJob(vkImage::Texture* texture, vkImage::TextureInput texInput) {
		this->texture = texture;
		this->texInfo = texInput;
	}

	void LoadTextureJob::execute(vk::CommandBuffer commandBuffer, vk::Queue queue) {
		texInfo.commandBuffer = commandBuffer;
		texInfo.queue = queue;
		texture->load(texInfo);
		status = JobStatus::FINISHED;
	}

	// TODO: Move this stuff to a separate class mayhaps
	void JobQueue::add(Job* job) {
		lock.lock();
		jobQueue.emplace(job);
		finished = false;
		lock.unlock();
	}

	Job* JobQueue::getNextJob() {
		// send this back as a flag if no jobs remain
		lock.lock();
		if (jobQueue.empty()) {
			finished = true;
			lock.unlock();
			return nullptr;
		}

		Job* nextJob = jobQueue.front();
		jobQueue.pop();
		lock.unlock();
		return nextJob;	
	}

	void JobQueue::clearQueue() {
		lock.lock();
		jobQueue = {};
		lock.unlock();
	}
}