#pragma once

#include <vector>
#include <algorithm>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/location.hpp>
#include <glm/vec2.hpp>

#include "coordinate_helper.hpp"

class Node
{
private:
	osmium::object_id_type _id;
	glm::vec2 _location;

public:
	Node(const osmium::Node &node) : _id{node.id()},
		_location{CoordinateHelper::latLonToEPSG25833(node.location())} {}

	glm::vec2 getLocation() const {
		return _location;
	}
};

class Way
{
private:
	osmium::object_id_type _id;
	std::vector<Node> _nodes;

public:
	Way(const osmium::object_id_type id, std::vector<Node> nodes) : _id{id}, _nodes{nodes} {}

	std::vector<Node> getNodes() const {
		return _nodes;
	}
};