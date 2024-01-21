#include "WorkerThread.h"

namespace vkJob {
	WorkerThread::WorkerThread(
		JobQueue& jobQueue, 
		vk::CommandBuffer commandBuffer, 
		vk::Queue queue
		) : jobQueue(jobQueue) {
		this->commandBuffer = commandBuffer;
		this->queue = queue;
		}

	void WorkerThread::operator()() {
		Job* nextJob = jobQueue.getNextJob();
		while (nextJob) {
			nextJob->execute(commandBuffer, queue);
			nextJob = jobQueue.getNextJob();
		}
	}
}