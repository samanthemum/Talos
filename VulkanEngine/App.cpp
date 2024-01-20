#include "App.h"

void App::buildGlfwWindow(int width, int height, bool debugMode) {
	// initialize glfw
	glfwInit();

	// set window hints
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// create the window
	if (window = glfwCreateWindow(width, height, "Vulkan Engine", nullptr, nullptr)) {
		if (debugMode) {
			std::cout << "Window was made successfully!" << std::endl;
		}
	}
	else {
		if (debugMode) {
			std::cout << "Window creation was unsuccessful!" << std::endl;
		}
	}

	// get glfw version
	if (debugMode) {
		std::cout << "GLFW version is " << glfwGetVersionString() << std::endl;
		std::cout << "Can GLFW support Vulkan? " << glfwVulkanSupported() << std::endl;
	}
}

void App::calculateFrameRate() {
	currentTime = glfwGetTime();
	double deltaTime = currentTime - lastTime;

	if (deltaTime >= 1) {
		int framerate{ std::max(1, int(numFrames / deltaTime)) };
		std::stringstream title;
		title << "Running at " << framerate << " fps.";
		glfwSetWindowTitle(window, title.str().c_str());

		lastTime = currentTime;
		numFrames = -1;
		frameTime = float(1000.0 / framerate);
	}

	numFrames++;
}


void App::run() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		graphicsEngine->render(scene);
		calculateFrameRate();
	}
}

App::App(int width, int height, bool debugMode) {
	buildGlfwWindow(width, height, debugMode);

	graphicsEngine = new Engine(width, height, window, debugMode);
	scene = new Scene("scenes/sample.txt");
}

App::~App() {
	delete graphicsEngine;
	delete scene;
	// delete window;
}

