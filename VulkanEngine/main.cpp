#include "App.h"

int main() {
	std::cout << "Hello Vulkan!" << std::endl;

	App* application = new App(1280, 960, true);
	application->run();
	delete application;

	return 0;
}