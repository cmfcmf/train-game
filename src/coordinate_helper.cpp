#include <GeographicLib/UTMUPS.hpp>

#include "coordinate_helper.hpp"

glm::vec2 CoordinateHelper::latLonToEPSG25833(const glm::vec2 &latLon)
{
	int zone;
	bool northp;
	double x, y, gamma, k;
	GeographicLib::UTMUPS::Forward(latLon.x, latLon.y, zone, northp, x, y, gamma, k, 33);
	return glm::vec2(x, y);
}

glm::vec2 CoordinateHelper::latLonToEPSG25833(const osmium::Location &location)
{
	return latLonToEPSG25833(glm::vec2(
		location.lat(),
		location.lon()));
}