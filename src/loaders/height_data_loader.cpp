#include "height_data_loader.hpp"

#include "../xyz_reader.hpp"

void HeightDataLoader::loadChunk(std::vector<float> &result, const std::string &filename, const std::uint32_t originX, const std::uint32_t originY,
	std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const {
	const XYZReader reader(filename);
	const auto data = reader.read();
	for (const auto &[absoluteX, absoluteY, z] : data) {
		const std::uint32_t x = static_cast<std::uint32_t>(absoluteX) - requestedX;
		const std::uint32_t y = static_cast<std::uint32_t>(absoluteY) - requestedY;
		if (x < 0 || y < 0) {
			continue;
		}
		if (x > extent || y > extent) {
			continue;
		}
		result[x + y * (extent + 1)] = z;
	}
}