#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "loader.hpp"

typedef std::vector<float> HeightDataLoaderResult;
typedef std::vector<std::tuple<float, float, float>> HeightDataLoaderRawData;

class HeightDataLoader : public Loader<HeightDataLoaderResult, HeightDataLoaderRawData>
{
protected:
	std::uint32_t getChunkSize() const override { return 2000; };
	std::string getName() const override { return "dgm"; }
	std::string getZipName() const override { return "dgm_{}.zip"; }
	std::string getRawName() const override { return "dgm_{}.xyz"; }
	std::string getDownloadUrl() const override { return "https://data.geobasis-bb.de/geobasis/daten/dgm/xyz/"; }

	HeightDataLoaderResult initResult(__attribute__((unused)) const std::uint32_t requestedX, __attribute__((unused)) const std::uint32_t requestedY, const std::uint16_t extent) const override {
		HeightDataLoaderResult result((extent + 1) * (extent + 1));
		return result;
	}

	HeightDataLoaderRawData parseFile(const std::string &filename) const;

	void loadChunk(HeightDataLoaderResult &result, const HeightDataLoaderRawData &rawData,
		const std::uint32_t originX, const std::uint32_t originY,
		const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override;
};