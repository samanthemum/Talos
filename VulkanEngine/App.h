#pragma once
#include "config.h"
#include "Engine.h"
#include "Scene.h"

class App {
	private:
		Engine* graphicsEngine;
		GLFWwindow* window;
		Scene* scene;

		double lastTime, currentTime;
		int numFrames;
		float frameTime;

		void buildGlfwWindow(int width, int height, bool debugMode);
		void calculateFrameRate();

	public:
		App(int widht, int height, bool debugMode);
		~App();
		void run();

};