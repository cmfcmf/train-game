#include <tuple>
#include <string>
#include <algorithm>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include "../vendor/stb/stb_image.h"

#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/factory.hpp>

#include "main.hpp"
#include "game.hpp"
#include "xyz_reader.hpp"
#include "config.hpp"
#include "osm_element.hpp"
#include "coordinate_helper.hpp"
#include "citygml_reader.hpp"
#include "chunk.hpp"
#include "loaders/height_data_loader.hpp"
#include "loaders/satelite_image_loader.hpp"

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

float randomBetween0And1()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

class MyOSMHandler : public osmium::handler::Handler
{
	const float _minX, _maxX, _minY, _maxY;

	std::map<osmium::object_id_type, Node> _nodes;
	std::vector<Way> _ways;

	bool processedWay = false;

public:
	MyOSMHandler(const float minX, const float maxX, const float minY, const float maxY) : _minX{minX}, _maxX{maxX}, _minY{minY}, _maxY{maxY} {}

	void node(const osmium::Node &node)
	{
		if (processedWay) {
			throw std::runtime_error("Must process all nodes before processing ways");
		}
		if (node.deleted())
		{
			return;
		}

		/*
		const auto &tags = node.tags();
		const auto &railwayTag = tags.get_value_by_key("railway");
		const auto isSignal = railwayTag != nullptr && std::strcmp(railwayTag, "signal") == 0;
		if (!isSignal && !tags.empty())
		{
			return;
		}
		*/

		//if (isInBounds(node.location())) {
			_nodes.insert({node.id(), Node(node)});
		//}
	}

	void way(const osmium::Way &way)
	{
		processedWay = true;
		if (way.deleted())
		{
			return;
		}

		const auto &tags = way.tags();
		const auto &railwayTag = tags.get_value_by_key("railway");
		const auto isProperRail = railwayTag != nullptr && (std::strcmp(railwayTag, "rail") == 0 || std::strcmp(railwayTag, "light_rail") == 0 || std::strcmp(railwayTag, "tram") == 0);
		if (!isProperRail)
		{
			return;
		}

		const auto &refs = way.nodes();
		std::vector<Node> nodes;
		nodes.reserve(refs.size());
		for (const auto &ref : refs) {
			const auto &result = _nodes.find(ref.ref());
			if (result != _nodes.end()) {
				nodes.push_back(result->second);
			} else {
				throw std::runtime_error("Could not find node");
			}
		}
		if (!nodes.empty()) {
			_ways.push_back(Way(way.id(), nodes));
		}
	}

	std::pair<std::map<osmium::object_id_type, Node>, std::vector<Way>> get() {
		return std::make_pair(_nodes, _ways);
	}

private:
	bool isInBounds(const osmium::Location &location)
	{
		if (!location.valid())
		{
			return false;
		}
		const auto &result = CoordinateHelper::latLonToEPSG25833(location);
		const auto x = result.x;
		const auto y = result.y;
		return x >= _minX && y >= _minY && x <= _maxX && y <= _maxY;
	}
};

int main()
{
	std::cout << "TrainGame " << TrainGame_VERSION_MAJOR << "." << TrainGame_VERSION_MINOR << "." << TrainGame_VERSION_PATCH << std::endl;

	const auto minX = 368000.0f;
	const auto minY = 5806000.0f;
	const auto extent = 2000;
	const auto maxX = minX + extent;
	const auto maxY = minY + extent;

	auto heightDataLoader = HeightDataLoader();
	auto sateliteImageLoader = SateliteImageLoader();
	auto buildingsLoader = BuildingsLoader();
	auto chunk = Chunk({ minX, minY }, extent);
	chunk.load(heightDataLoader, sateliteImageLoader, buildingsLoader);

	const auto x = static_cast<size_t>(minX / 1000.0f);
	const auto y = static_cast<size_t>(minY / 1000.0f);
	const auto initialCameraPosition = glm::vec2(-minX, -minY);

	const auto osmFilePath = "datasets/osm/brandenburg-latest.osm.pbf";
	osmium::io::Reader osmReader{osmFilePath, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
	MyOSMHandler osmHandler(minX, maxX, minY, maxY);
	osmium::apply(osmReader, osmHandler);
	osmReader.close();
	const auto &[osmNodes, osmWays] = osmHandler.get();

	std::cout << "Found " << osmNodes.size() << " OSM nodes." << std::endl;
	std::cout << "Found " << osmWays.size() << " OSM ways." << std::endl;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const auto &heightData = chunk.getHeightData();
	for (auto i = 0; i < (extent + 1) * (extent + 1); i++)
	{
		float x = minX + i % (extent + 1);
		float y = minY + i / (extent + 1);
		float z = heightData[i];

		Vertex vertex = {
			{-x, -y, z},
			{0.373f, 0.608f, 0.106f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f}};
		const auto color = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		vertex.color *= 1.0f - 0.05f + (0.10 * color);
		vertices.push_back(vertex);
	}

	std::cout << "Generated vertices" << std::endl;

	for (size_t row = 0; row < extent; row++)
	{
		for (size_t column = 0; column < extent; column++)
		{
			const auto tl = column + row * (extent + 1);
			const auto tr = tl + 1;
			const auto bl = column + (row + 1) * (extent + 1);
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
				column / (extent + 1.0f),
				row / (extent + 1.0f)};
		}
	}

	std::cout << "Generated indices" << std::endl;


	for (const auto &way : osmWays) {
		const auto &nodes = way.getNodes();
		for (size_t i = 0; i < nodes.size() - 1; i++) {
			const auto &from = nodes[i];
			const auto &to = nodes[i + 1];

			const auto &start = from.getLocation();
			const auto &end = to.getLocation();
			const auto &segment = end - start;
			const auto &segment90 = glm::normalize(glm::vec2(-segment.y, segment.x));

			const auto gauge = 1.435f;

			const std::array<glm::vec2, 4> positions = {
				start - segment90 * (gauge / 2),
				start + segment90 * (gauge / 2),
				end - segment90 * (gauge / 2),
				end + segment90 * (gauge / 2),
			};
			const auto offset = vertices.size();
			const auto color = glm::vec3(1.0f, randomBetween0And1(), 0.0f);
			for (const auto &position : positions) {
				vertices.push_back({
					{-position.x, -position.y, 35},
					color,
					{0.0f, 0.0f, 1.0f},
					{0.0f, 0.0f}
				});
			}

			indices.push_back(offset + 0);
			indices.push_back(offset + 2);
			indices.push_back(offset + 1);

			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
		}
	}

	const auto &[buildingVertices, buildingIndices] = chunk.getBuildings();
	const auto buildingVertexOffset = vertices.size();
	for (const auto &vertex : buildingVertices) {
		vertices.push_back({vertex.position, vertex.color, vertex.normal, {0.0f, 0.0f}});
	}
	for (const auto &index : buildingIndices) {
		indices.push_back(buildingVertexOffset + index);
	}

	TrainGameApplication app(vertices, indices, chunk.getSateliteImage(), initialCameraPosition);

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}