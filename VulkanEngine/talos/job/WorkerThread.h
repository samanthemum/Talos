#pragma once
#include "Job.h"

namespace vkJob {
	class WorkerThread {
		public:
			JobQueue& jobQueue;
			vk::CommandBuffer commandBuffer;
			vk::Queue queue;

			WorkerThread(
				JobQueue& jobQueue, vk::CommandBuffer commandBuffer, vk::Queue queue
			);

			void operator()();
	};
}