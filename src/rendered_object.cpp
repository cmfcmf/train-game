#include "rendered_object.hpp"


const std::vector<Vertex> RenderedObject::getVertices() const
{
	return m_vertices;
}

const std::vector<uint32_t> RenderedObject::getIndices() const
{
	return m_indices;
}

const std::vector<uint32_t> RenderedObject::getTexture() const
{
	return m_texture;
}