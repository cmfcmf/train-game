#include <tuple>
#include <string>
#include <algorithm>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
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
		const auto isProperRail = railwayTag != nullptr && std::strcmp(railwayTag, "rail") == 0;
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

	const auto minX = 372000.0f;
	const auto maxX = 374000.0f;
	const auto minY = 5822000.0f;
	const auto maxY = 5824000.0f;

	const auto x = static_cast<size_t>(minX / 1000.0f);
	const auto y = static_cast<size_t>(minY / 1000.0f);

	const auto heightMapFilePath = "dataset/dgm/raw/dgm_33" + std::to_string(x) + "-" + std::to_string(y) + ".xyz";
	const auto textureImageFilePath = "dataset/dop/raw/dop_33" + std::to_string(x) + "-" + std::to_string(y) + ".jpg";
	const auto osmFilePath = "dataset/osm/brandenburg-latest.osm.pbf";

	const auto initialCameraPosition = glm::vec2(-minX, -minY);

	osmium::io::Reader osmReader{osmFilePath, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
	MyOSMHandler osmHandler(minX, maxX, minY, maxY);
	osmium::apply(osmReader, osmHandler);
	osmReader.close();
	const auto &[osmNodes, osmWays] = osmHandler.get();

	std::cout << "Found " << osmNodes.size() << " OSM nodes." << std::endl;
	std::cout << "Found " << osmWays.size() << " OSM ways." << std::endl;

	const size_t NUM_ROWS = 2001;

	const auto heightMapReader = XYZReader(heightMapFilePath);
	const auto result = heightMapReader.read(NUM_ROWS);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto &each : result)
	{
		Vertex vertex = {
			{-std::get<0>(each), -std::get<1>(each), std::get<2>(each)},
			// { randomBetween0And1(), randomBetween0And1(), randomBetween0And1() },
			{0.373f, 0.608f, 0.106f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f}};
		const auto color = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		vertex.color *= 1.0f - 0.05f + (0.10 * color);
		vertices.push_back(vertex);
	}

	const auto [min, max] = std::minmax_element(vertices.begin(), vertices.end(), [](const auto &a, const auto &b) {
		return a.pos.z < b.pos.z;
	});

	std::cout << "min: " << min->pos.z << " max: " << max->pos.z << std::endl;

	std::cout << "Generated vertices" << std::endl;

	for (size_t row = 0; row < NUM_ROWS - 1; row++)
	{
		for (size_t column = 0; column < 2000; column++)
		{
			const auto tl = column + row * 2001;
			const auto tr = tl + 1;
			const auto bl = column + (row + 1) * 2001;
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
				column / 2001.0f,
				row / -2001.0f};
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

	// std::cout << indices[0] << std::endl
	// 		  << indices[1] << std::endl
	// 		  << indices[2] << std::endl;
	// std::cout << vertices[2001].pos.x << std::endl
	// 		  << vertices[2001].pos.y << std::endl;

	// vertices[0].color = {1.0f, 0.0f, 0.0f};
	// vertices[vertices.size() - 1].color = {0.0f, 0.0f, 1.0f};

	/*
	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

		{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};

	const std::vector<uint32_t> indices = {
		0, 1, 2,
		2, 3, 0,
		1, 4, 2
	};
*/

	TrainGameApplication app(vertices, indices, textureImageFilePath, initialCameraPosition);

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