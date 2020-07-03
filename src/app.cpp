#include "app.hpp"

#include <memory>
#include <future>

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

std::vector<ChunkID> App::calculateRequiredChunkIds() {
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


void App::synchronizeChunks()
{
	// https://fbinter.stadt-berlin.de/fb/wms/senstadt/k_luftbild2019_rgb?service=wms&request=GetMap&layers=0&styles=default&srs=EPSG:25833&bbox=373000,5810000,374000,5812000&width=5000&height=5000&format=image/jpeg
	// https://fbinter.stadt-berlin.de/fb/wms/senstadt/k_dgm1/?service=wms&request=GetMap&layers=1&styles=default&srs=EPSG:25833&bbox=373000,5810000,374000,5812000&width=5000&height=5000&format=image/jpeg

	if (chunkLoadingFuture) {
		if (chunkLoadingFuture->wait_for(std::chrono::microseconds(0)) != std::future_status::ready) {
			// Chunk loading is in progress.
			return;
		} else {
			// Chunk loading is done.
			const auto newChunks = chunkLoadingFuture->get();
			chunkLoadingFuture.reset();

			for (const auto &chunk : newChunks) {
				m_chunks.insert({chunk.getId(), chunk});
			}

			std::vector<RenderedObject> objects(m_additionalRenderedObjects.begin(), m_additionalRenderedObjects.end());
			for (const auto &chunk : m_chunks) {
				objects.insert(objects.end(), chunk.second.getRenderedObjects().begin(), chunk.second.getRenderedObjects().end());
			}
			renderer->setRenderedObjects(objects);

			return;
		}
	}

	const auto requiredChunkIDs = calculateRequiredChunkIds();

	std::set<ChunkID> chunkIDsToUnload;
	for (const auto &[chunkID, chunk] : m_chunks) {
		chunkIDsToUnload.insert(chunkID);
	}

	std::set<ChunkID> chunkIDsToLoad;
	for (const auto &chunkID : requiredChunkIDs) {
		if (chunkIDsToUnload.erase(chunkID) > 0) {
			// chunk is already loaded
			continue;
		}
		chunkIDsToLoad.insert(chunkID);
	}

	if (!chunkIDsToLoad.empty() || !chunkIDsToUnload.empty()) {
		for (const auto &chunkId : chunkIDsToUnload) {
			m_chunks.erase(chunkId);
		}

		chunkLoadingFuture = std::async(std::launch::async, [](const std::set<ChunkID> chunkIDsToLoad, HeightDataLoader &heightDataLoader, SateliteImageLoader &sateliteImageLoader, BuildingsLoader &buildingsLoader) {
			std::vector<Chunk> chunks;
			for (const auto &chunkID : chunkIDsToLoad) {
				auto chunk = Chunk::fromChunkID(chunkID);
				chunk.load(heightDataLoader, sateliteImageLoader, buildingsLoader);
				chunks.push_back(chunk);
			}

			return chunks;
		}, chunkIDsToLoad, std::ref(heightDataLoader), std::ref(sateliteImageLoader), std::ref(buildingsLoader));
	}
}

void App::run()
{
	m_additionalRenderedObjects.push_back(loadOSMData());

	initWindow();
	renderer = std::make_unique<Renderer>(window);
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

		m_position.x += 1.0f * delta * horizontalTranslation * zoom;
		m_position.y += 1.0f * delta * verticalTranslation * zoom;
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

	spdlog::trace("x: {}, y: {}, z: {}", cameraPosition.x, cameraPosition.y, cameraPosition.z);
	renderer->setCameraMatrix(mat);

	synchronizeChunks();
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