#pragma once

#include <optional>
#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
    glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};