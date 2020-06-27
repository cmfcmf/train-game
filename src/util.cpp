#include "util.hpp"

const auto VERTEX_BUFFER_BIND_ID = 0;

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = VERTEX_BUFFER_BIND_ID;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

	attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 vertices
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 normals
	attributeDescriptions[2].offset = offsetof(Vertex, normal);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}
