#pragma once

#include <glm/vec2.hpp>
#include <osmium/osm/location.hpp>


class CoordinateHelper {
public:
	static glm::vec2 latLonToEPSG25833(const glm::vec2 &latLon);
	static glm::vec2 latLonToEPSG25833(const osmium::Location &location);
};