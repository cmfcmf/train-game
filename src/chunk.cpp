#include "chunk.hpp"
#include <cassert>
#include "spdlog/spdlog.h"

void adjustNormal(std::vector<Vertex> &vertices, size_t a, size_t b, size_t c)
{
	auto &A = vertices[a];
	auto &B = vertices[b];
	auto &C = vertices[c];

	glm::vec3 normal = glm::normalize(glm::cross(B.pos - A.pos, C.pos - A.pos));
	// normal *= -1;
	// std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

	A.normal += normal;
	B.normal += normal;
	C.normal += normal;
}

void Chunk::load(HeightDataLoader &heightDataLoader, SateliteImageLoader &sateliteImageLoader, BuildingsLoader &buildingsLoader) {
	spdlog::info("Chunk loading started.");

	spdlog::debug("Loading height data.");
	heightData = heightDataLoader.load(_origin.x, _origin.y, _extent);
	spdlog::debug("Loading satelite image data.");
	sateliteImage = sateliteImageLoader.load(_origin.x, _origin.y, _extent);
	spdlog::debug("Loading buildings data.");
	buildings = buildingsLoader.load(_origin.x, _origin.y, _extent);

	spdlog::debug("Transforming data.");
	transformHeightData();
	transformBuildings();

	spdlog::info("Chunk loading finished.");
}

void Chunk::transformHeightData()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (auto i = 0; i < (_extent + 1) * (_extent + 1); i++)
	{
		float x = _origin.x + i % (_extent + 1);
		float y = _origin.y + i / (_extent + 1);
		float z = heightData[i];

		Vertex vertex = {
			{x, y, z},
			{0.373f, 0.608f, 0.106f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}};
		const auto color = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		vertex.color *= 1.0f - 0.05f + (0.10 * color);
		vertices.push_back(vertex);
	}

	for (size_t row = 0; row < _extent; row++)
	{
		for (size_t column = 0; column < _extent; column++)
		{
			const auto tl = column + row * (_extent + 1);
			const auto tr = tl + 1;
			const auto bl = column + (row + 1) * (_extent + 1);
			const auto br = bl + 1;

			indices.push_back(tr);
			indices.push_back(bl);
			indices.push_back(tl);
			adjustNormal(vertices, tr, bl, tl);

			indices.push_back(tr);
			indices.push_back(br);
			indices.push_back(bl);
			adjustNormal(vertices, tr, br, bl);

			vertices[tl].texCoord = {
				column / (_extent + 1.0f),
				row / (_extent + 1.0f)};
		}
	}

	m_renderedObjects.push_back(RenderedObject(std::move(vertices), std::move(indices), sateliteImage));
}

void Chunk::transformBuildings()
{
	const auto &[buildingVertices, indices] = buildings;
	std::vector<Vertex> vertices(buildingVertices.size());

	size_t i = 0;
	for (const auto &vertex : buildingVertices) {
		vertices.at(i++) = {vertex.position, vertex.color, vertex.normal, {0.0f, 0.0f}};
	}

	m_renderedObjects.push_back(RenderedObject(std::move(vertices), std::move(indices)));
}