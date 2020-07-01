#include "citygml_reader.hpp"

#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>
#include <citygml/cityobject.h>
#include <citygml/geometry.h>
#include <citygml/polygon.h>

void gatherGeometries(std::vector<citygml::Geometry> &geometries, const citygml::Geometry &geometry) {
	geometries.push_back(geometry);
	for (size_t g = 0; g < geometry.getGeometriesCount(); g++)
	{
		const auto &childGeometry = geometry.getGeometry(g);
		gatherGeometries(geometries, childGeometry);
	}
}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> CityGMLReader::read() const
{
	std::cout << "Loading file " << _filename << std::endl;
	citygml::ParserParams params;

	std::shared_ptr<const citygml::CityModel> city;
	try
	{
		city = citygml::load(_filename, params);
	}
	catch (const std::runtime_error &e)
	{
		throw std::runtime_error("Could not parse file " + _filename);
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	size_t vertexOffset = 0;

	for (const auto &object : city->getRootCityObjects())
	{
		if (object->getType() != citygml::CityObject::CityObjectsType::COT_Building)
		{
			continue;
		}
		std::cout << object->getId() << " : " << object->getGeometriesCount() << std::endl;

		std::vector<citygml::Geometry> geometries;

		for (size_t g = 0; g < object->getGeometriesCount(); g++)
		{
			const auto &geometry = object->getGeometry(g);
			gatherGeometries(geometries, geometry);
		}
		for (const auto &geometry : geometries) {
			for (size_t p = 0; p < geometry.getPolygonsCount(); p++)
			{
				const auto &polygon = geometry.getPolygon(p);
				for (const auto &index : polygon->getIndices())
				{
					indices.push_back(vertexOffset + index);
				}
				for (const auto &vertex : polygon->getVertices())
				{
					const auto &normal = vertex.normal();
					vertices.push_back({glm::vec3(vertex.x, vertex.y, vertex.z),
										glm::vec3(192.0f / 0xFF, 192.0f / 0xFF, 192.0f / 0xFF),
										glm::vec3(normal.x, normal.y, normal.z),
										glm::vec2(0.0f, 0.0f)});
					vertexOffset++;
				}
			}
		}
	}

	return std::make_pair(vertices, indices);
}