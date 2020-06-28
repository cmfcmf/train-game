
#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "loader.hpp"
#include "../util.hpp"

struct BuildingVertex {
	const glm::vec3 position;
	const glm::vec3 normal;
	const glm::vec3 color;
};

typedef std::pair<std::vector<BuildingVertex>, std::vector<std::uint32_t>> BuildingsLoaderResult;

class BuildingsLoader : public Loader<BuildingsLoaderResult>
{
protected:
	std::uint32_t getChunkSize() const override { return 1000; };
	std::string getName() const override { return "3d_gebaeude_lod2"; }
	std::string getZipName() const override { return "lod2_{}.zip"; }
	std::string getRawName() const override { return "lod2_{}_geb.gml"; }
	std::string getDownloadUrl() const override { return "https://data.geobasis-bb.de/geobasis/daten/3d_gebaeude/lod2_gml/"; }

	BuildingsLoaderResult initResult(const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override {
		BuildingsLoaderResult result;
		return result;
	}

	void loadChunk(BuildingsLoaderResult &result, const std::string &filename,
		const std::uint32_t originX, const std::uint32_t originY,
		const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const override;
};