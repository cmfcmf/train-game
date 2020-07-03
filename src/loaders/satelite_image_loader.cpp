
#include "satelite_image_loader.hpp"

#include <stb_image.h>

#include "../exception.hpp"

SateliteImageLoaderRawData SateliteImageLoader::parseFile(const std::string &filename) const {
	int texWidth, texHeight, texChannels;
    std::uint32_t* pixels = reinterpret_cast<std::uint32_t*>(stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));
    if (!pixels) {
        throw_with_trace(std::runtime_error("failed to load image: " + filename));
    }
	assert(static_cast<std::uint32_t>(texWidth) == IMAGE_SIZE);
	assert(static_cast<std::uint32_t>(texHeight) == IMAGE_SIZE);

	auto vec = SateliteImageLoaderRawData(pixels, pixels + IMAGE_SIZE * IMAGE_SIZE);

    stbi_image_free(pixels);

	return vec;
}


void SateliteImageLoader::loadChunk(SateliteImageLoaderResult &result, const SateliteImageLoaderRawData &rawData,
	const std::uint32_t originX, const std::uint32_t originY,
	const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const {

	for (size_t i = 0; i < IMAGE_SIZE * IMAGE_SIZE; i++) {
		const auto pixel = rawData[i];
		const auto absoluteX = originX * PIXELS_PER_METER + (i % IMAGE_SIZE);
		// The origin of the image is a the top left, while the origin of the chunk is at the bottom left.
		const auto absoluteY = originY * PIXELS_PER_METER + (IMAGE_SIZE - 1 - (i / IMAGE_SIZE));

		const std::int32_t x = static_cast<std::int32_t>(absoluteX) - requestedX * PIXELS_PER_METER;
		const std::int32_t y = static_cast<std::int32_t>(absoluteY) - requestedY * PIXELS_PER_METER;
		if (x < 0 || y < 0) {
			continue;
		}
		if (x >= static_cast<std::int32_t>(extent * PIXELS_PER_METER) || y >= static_cast<std::int32_t>(extent * PIXELS_PER_METER)) {
			continue;
		}
		result.at(x + y * extent * PIXELS_PER_METER) = pixel;
	}
}