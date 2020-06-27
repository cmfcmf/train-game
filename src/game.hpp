#pragma once

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>
#include <set>
#include <optional>
#include <fstream>
#include <chrono>
#include <map>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "keyboard.hpp"
#include "util.hpp"

class TrainGameApplication
{
public:
	TrainGameApplication(std::vector<Vertex> vertices, std::vector<uint32_t> indices) : vertices{vertices}, indices{indices} {}

	void run();
	Keyboard keyboard;

	static TrainGameApplication *fromWindow(GLFWwindow *window);

private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation"};

	const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	//	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	GLFWwindow *window;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

	void initWindow();

	static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	void initVulkan();

	void createInstance();

	std::vector<const char *> getRequiredExtensions();
	void printAvailableExtensions();
	bool checkValidationLayerSupport();

	void setupDebugMessenger();
	void createSurface();

	void pickPhysicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

	void createSwapChain();

	void createImageViews();

	void createRenderPass();

	void createDescriptorSetLayout();

	void createGraphicsPipeline();

	VkShaderModule createShaderModule(const std::vector<char> &code);

	void createFramebuffers();

	void createCommandPool();

	VkImageView createImageView(VkImage image, VkFormat format);

	void createTextureImage();

	void createTextureImageView();

	void createTextureSampler();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);


	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void createVertexBuffer();

	void createIndexBuffer();

	void createUniformBuffers();

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createDescriptorPool();

	void createDescriptorSets();

	void createCommandBuffers();

	void createSyncObjects();

	void recreateSwapChain();

	void mainLoop();

	void drawFrame();

	void updateUniformBuffer(uint32_t currentImage);

	void cleanup();

	void cleanupSwapChain();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData);
};
