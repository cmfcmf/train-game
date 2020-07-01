
#include "buildings_loader.hpp"
#include "../citygml_reader.hpp"

BuildingsLoaderRawData BuildingsLoader::parseFile(const std::string &filename) const {
	CityGMLReader cityGMLReader(filename);
	return cityGMLReader.read();
}

void BuildingsLoader::loadChunk(BuildingsLoaderResult &result, const BuildingsLoaderRawData &rawData,
	__attribute__((unused)) const std::uint32_t originX, __attribute__((unused)) const std::uint32_t originY,
	__attribute__((unused)) const std::uint32_t requestedX, __attribute__((unused)) const std::uint32_t requestedY, __attribute__((unused)) const std::uint16_t extent) const {
	// FIXME: This completely ignores extent and requestedX/requestedY.
	const auto &[buildingVertices, buildingIndices] = rawData;

	const auto indexOffset = result.first.size();
	for (const auto &vertex : buildingVertices) {
		result.first.push_back({vertex.pos, vertex.normal, vertex.color});
	}
	for (const auto index : buildingIndices) {
		result.second.push_back(index + indexOffset);
	}
}