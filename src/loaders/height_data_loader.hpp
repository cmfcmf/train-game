#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "loader.hpp"

class HeightDataLoader : public Loader<std::vector<float>>
{
protected:
	std::uint32_t getChunkSize() const override { return 2000; };
	std::string getName() const override { return "dgm"; }
	std::string getZipName() const override { return "dgm_{}.zip"; }
	std::string getRawName() const override { return "dgm_{}.xyz"; }
	std::string getDownloadUrl() const override { return "https://data.geobasis-bb.de/geobasis/daten/dgm/xyz/"; }

	std::vector<float> initResult(const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override {
		std::vector<float> result((extent + 1) * (extent + 1));
		return result;
	}

	void loadChunk(std::vector<float> &result, const std::string &filename, const std::uint32_t originX, const std::uint32_t originY,
		const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override;
};