
#include "satelite_image_loader.hpp"

#include "../../vendor/stb/stb_image.h"

void SateliteImageLoader::loadChunk(std::vector<uint32_t> &result, const std::string &filename,
	const std::uint32_t originX, const std::uint32_t originY,
	const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const {

	int texWidth, texHeight, texChannels;
    std::uint32_t* pixels = reinterpret_cast<std::uint32_t*>(stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));
    if (!pixels) {
        throw std::runtime_error("failed to load image: " + filename);
    }
	assert(texWidth == IMAGE_SIZE);
	assert(texHeight == IMAGE_SIZE);

	for (auto i = 0; i < IMAGE_SIZE * IMAGE_SIZE; i++) {
		const auto pixel = pixels[i];
		const auto absoluteX = originX * PIXELS_PER_METER + (i % IMAGE_SIZE);
		// The origin of the image is a the top left, while the origin of the chunk is at the bottom left.
		const auto absoluteY = originY * PIXELS_PER_METER + (IMAGE_SIZE - 1 - (i / IMAGE_SIZE));

		const auto x = absoluteX - requestedX * PIXELS_PER_METER;
		const auto y = absoluteY - requestedY * PIXELS_PER_METER;
		if (x < 0 || y < 0) {
			continue;
		}
		if (x >= extent * PIXELS_PER_METER || y >= extent * PIXELS_PER_METER) {
			continue;
		}
		result.at(x + y * extent * PIXELS_PER_METER) = pixel;
	}
    stbi_image_free(pixels);
}