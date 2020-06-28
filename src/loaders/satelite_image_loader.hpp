
#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "loader.hpp"

class SateliteImageLoader : public Loader<std::vector<uint32_t>>
{
protected:
	std::uint32_t getChunkSize() const override { return 2000; };
	std::string getName() const override { return "dop"; }
	std::string getZipName() const override { return "dop_{}.zip"; }
	std::string getRawName() const override { return "dop_{}.jpg"; }
	std::string getDownloadUrl() const override { return "https://data.geobasis-bb.de/geobasis/daten/dop/rgb_jpg/"; }

	const std::uint32_t IMAGE_SIZE = 10000;
	const std::uint32_t PIXELS_PER_METER = IMAGE_SIZE / 2000;

	std::vector<uint32_t> initResult(const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override {
		std::vector<uint32_t> result(PIXELS_PER_METER * PIXELS_PER_METER * extent * extent);
		return result;
	}

	void loadChunk(std::vector<uint32_t> &result, const std::string &filename,
		const std::uint32_t originX, const std::uint32_t originY,
		const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override;
};