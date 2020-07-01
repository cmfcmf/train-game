#pragma once

#include <vector>

#include "util.hpp"

class RenderedObject
{
private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::vector<uint32_t> m_texture;

public:
	RenderedObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<uint32_t> &texture = {}) :
		m_vertices{vertices}, m_indices{indices}, m_texture{texture} {}

	const std::vector<Vertex> getVertices() const;
	const std::vector<uint32_t> getIndices() const;
	const std::vector<uint32_t> getTexture() const;
};