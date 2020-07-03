#pragma once

#include <GLFW/glfw3.h>

#include "keyboard.hpp"
#include "renderer.hpp"
#include "chunk.hpp"
#include "rendered_object.hpp"

class App
{
public:

	static App *fromWindow(GLFWwindow *window);
	Keyboard keyboard;

	void run();

private:
	std::shared_ptr<GLFWwindow> window;
	std::unique_ptr<Renderer> renderer;

	std::vector<RenderedObject> m_additionalRenderedObjects;
	std::map<ChunkID, Chunk> m_chunks;

	glm::vec2 m_position = {368000.0f, 5806000.0f};
	const uint32_t extent = 400;

	const int INITIAL_WINDOW_WIDTH = 800;
	const int INITIAL_WINDOW_HEIGHT = 600;

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	void loadData();

	void initWindow();

	void setWindowTitle(const std::string&);

	std::vector<ChunkID> calculateRequiredChunkIds();

	void mainLoop();

	void processInput();

	void destroyWindow();
};