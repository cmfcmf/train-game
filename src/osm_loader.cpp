#include "osm_loader.hpp"

#include <vector>
#include <string>
#include <iostream>

#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/factory.hpp>
#include <spdlog/spdlog.h>

#include "rendered_object.hpp"
#include "osm_element.hpp"
#include "util.hpp"
#include "exception.hpp"


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
			throw_with_trace(std::runtime_error("Must process all nodes before processing ways"));
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
				throw_with_trace(std::runtime_error("Could not find node"));
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


RenderedObject loadOSMData(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
{
	spdlog::info("Loading OSM data.");

	const auto osmFilePath = "datasets/osm/brandenburg-latest.osm.pbf";
	osmium::io::Reader osmReader{osmFilePath, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
	MyOSMHandler osmHandler(minX, maxX, minY, maxY);
	osmium::apply(osmReader, osmHandler);
	osmReader.close();
	const auto &[osmNodes, osmWays] = osmHandler.get();

	spdlog::debug("Found {} OSM nodes.", osmNodes.size());
	spdlog::debug("Found {} OSM ways.", osmWays.size());

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

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
					{position.x, position.y, 35},
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

	spdlog::info("Finished loading OSM data.");
	return RenderedObject(std::move(vertices), std::move(indices));
}