#include "app.hpp"

#include <memory>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "osm_loader.hpp"
#include "util.hpp"

glm::vec3 rotate(const glm::vec3 &vector, const float rotation, const glm::vec2 &origin) {
	const auto mat1 = glm::translate(glm::mat4(1.0f), glm::vec3(-origin.x, -origin.y, 0.0));
	const auto mat2 = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	const auto mat3 = glm::translate(glm::mat4(1.0f), glm::vec3(origin.x, origin.y, 0.0));
	return glm::vec3(mat3 * mat2 * mat1 * glm::vec4(vector, 1.0f));
}

App* App::fromWindow(GLFWwindow *window)
{
	return reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
}

void App::framebufferResizeCallback(GLFWwindow *window, __attribute__((unused)) int width, __attribute__((unused)) int height)
{
	auto app = fromWindow(window);
	app->renderer->framebufferResized = true;
}

std::vector<ChunkID> App::calculateRequiredChunkIds()
{	// FIXME: 20000000 index
 	// FIXME: 5000000 vertex
	const uint32_t viewDistance = 500;

	const glm::ivec2 bl(
		floorToMultipleOf(static_cast<uint32_t>(std::round(m_position.x - viewDistance)), extent),
		floorToMultipleOf(static_cast<uint32_t>(std::round(m_position.y - viewDistance)), extent)
	);
	const glm::ivec2 tr(
		ceilToMultipleOf(static_cast<uint32_t>(std::round(m_position.x + viewDistance)), extent),
		ceilToMultipleOf(static_cast<uint32_t>(std::round(m_position.y + viewDistance)), extent)
	);

	std::vector<ChunkID> chunkIDs;

	for (int x = bl.x; x < tr.x; x += extent) {
		for (int y = bl.y; y < tr.y; y += extent) {
			chunkIDs.emplace_back(x, y, extent);
		}
	}
	return chunkIDs;
}


void App::loadData()
{
	// https://fbinter.stadt-berlin.de/fb/wms/senstadt/k_luftbild2019_rgb?service=wms&request=GetMap&layers=0&styles=default&srs=EPSG:25833&bbox=373000,5810000,374000,5812000&width=5000&height=5000&format=image/jpeg
	// https://fbinter.stadt-berlin.de/fb/wms/senstadt/k_dgm1/?service=wms&request=GetMap&layers=1&styles=default&srs=EPSG:25833&bbox=373000,5810000,374000,5812000&width=5000&height=5000&format=image/jpeg

	auto heightDataLoader = HeightDataLoader();
	auto sateliteImageLoader = SateliteImageLoader();
	auto buildingsLoader = BuildingsLoader();

	const auto chunkIDs = calculateRequiredChunkIds();
	for (auto &chunkID : chunkIDs) {
		auto chunk = Chunk({ std::get<0>(chunkID), std::get<1>(chunkID) }, std::get<2>(chunkID));
		chunk.load(heightDataLoader, sateliteImageLoader, buildingsLoader);

		m_chunks.insert({chunk.getId(), chunk});
	}

	const auto maxX = m_position.x + extent;
	const auto maxY = m_position.y + extent;
	m_additionalRenderedObjects.push_back(loadOSMData(m_position.x, m_position.y, maxX, maxY));
}

void App::run()
{
	loadData();
	initWindow();

	std::vector<RenderedObject> objects(m_additionalRenderedObjects.begin(), m_additionalRenderedObjects.end());
	for (const auto &chunk : m_chunks) {
		objects.insert(objects.end(), chunk.second.getRenderedObjects().begin(), chunk.second.getRenderedObjects().end());
	}

	renderer = std::make_unique<Renderer>(window);
	renderer->setRenderedObjects(objects);
	renderer->initVulkan();
	mainLoop();
	renderer->cleanup();
	destroyWindow();
}

void App::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "TrainGame", nullptr, nullptr), glfwDestroyWindow);
	glfwSetWindowUserPointer(window.get(), this);
	glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

	keyboard.begin(window.get());
	keyboard.addTrackedKey(GLFW_KEY_Q);
	keyboard.addTrackedKey(GLFW_KEY_E);

	keyboard.addTrackedKey(GLFW_KEY_W);
	keyboard.addTrackedKey(GLFW_KEY_A);
	keyboard.addTrackedKey(GLFW_KEY_S);
	keyboard.addTrackedKey(GLFW_KEY_D);
}

void App::mainLoop()
{
	while (!glfwWindowShouldClose(window.get()))
	{
		glfwPollEvents();

		processInput();

		renderer->drawFrame();
	}
}

void App::processInput()
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;

	static auto rotation = 0.0f;
	const auto rotationDirection = keyboard.isOnePressed(GLFW_KEY_Q, GLFW_KEY_E);
	rotation += glm::radians(90.0f) * delta * rotationDirection;

	static auto cameraPosition = glm::vec3(0.0f, 0.0f, 100.0f);
	const auto zoom = cameraPosition.z;

	{
		const auto verticalTranslation = keyboard.isOnePressed(GLFW_KEY_W, GLFW_KEY_S);
		const auto horizontalTranslation = keyboard.isOnePressed(GLFW_KEY_D, GLFW_KEY_A);
		cameraPosition.x += 1.0f * delta * horizontalTranslation * zoom;
		cameraPosition.y += 1.0f * delta * verticalTranslation * zoom;
	}
	{
		const auto scroll = keyboard.getScrollOffset();
		keyboard.resetScrollOffset();
		const auto zChange = zoom * delta * scroll * -1;
		cameraPosition.z = std::max(0.1, cameraPosition.z + zChange);
		//cameraPosition.y += zChange;
	}

	const auto title = "TrainGame "
		+ std::to_string(static_cast<uint16_t>(std::round(1.0f / delta)))
		+ " FPS";
	setWindowTitle(title);

	const auto mat = glm::lookAt(
		glm::vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z), // eye
		rotate(glm::vec3(cameraPosition.x, cameraPosition.y + 100.0f, 0.0f), rotation, glm::vec2(cameraPosition.x, cameraPosition.y)), // look at
		glm::vec3(0.0f, 0.0f, 1.0f) // up
	);

	std::cout << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << std::endl;
	renderer->setCameraMatrix(mat);
}

void App::setWindowTitle(const std::string &title)
{
	glfwSetWindowTitle(window.get(), title.c_str());
}

void App::destroyWindow()
{
	renderer.reset();
	window.reset();
	glfwTerminate();
}