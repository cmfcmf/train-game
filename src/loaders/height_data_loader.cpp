#include "height_data_loader.hpp"

#include "../xyz_reader.hpp"

HeightDataLoaderRawData HeightDataLoader::parseFile(const std::string &filename) const {
	const XYZReader reader(filename);
	return reader.read();
}

void HeightDataLoader::loadChunk(HeightDataLoaderResult &result, const HeightDataLoaderRawData &rawData,
	__attribute__((unused)) const std::uint32_t originX, __attribute__((unused)) const std::uint32_t originY,
	const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const {
	for (const auto &[absoluteX, absoluteY, z] : rawData) {
		const std::int32_t x = static_cast<std::int32_t>(absoluteX) - requestedX;
		const std::int32_t y = static_cast<std::int32_t>(absoluteY) - requestedY;
		if (x < 0 || y < 0) {
			continue;
		}
		if (x > extent || y > extent) {
			continue;
		}
		result[x + y * (extent + 1)] = z;
	}
}