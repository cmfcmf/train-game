#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "loaders/height_data_loader.hpp"
#include "loaders/satelite_image_loader.hpp"
#include "loaders/buildings_loader.hpp"
#include "rendered_object.hpp"

typedef glm::vec<2, std::uint32_t, glm::defaultp> Vec2Uint32;
typedef std::tuple<uint32_t, uint32_t, uint16_t> ChunkID;

class Chunk
{
public:
	Chunk(Vec2Uint32 origin, std::uint16_t extent) : _origin{origin}, _extent{extent} {
		if (origin.x % 100 != 0) {
			throw std::runtime_error("origin.x must be a multiple of 100.");
		}
		if (origin.y % 100 != 0) {
			throw std::runtime_error("origin.y must be a multiple of 100.");
		}
		if (extent % 100 != 0) {
			throw std::runtime_error("extent must be a multiple of 100.");
		}
	}

	void load(HeightDataLoader&, SateliteImageLoader&, BuildingsLoader&);

	const std::vector<float>& getHeightData() const {
		return heightData;
	}

	const std::vector<std::uint32_t>& getSateliteImage() const {
		return sateliteImage;
	}

	const BuildingsLoaderResult& getBuildings() const {
		return buildings;
	}

	const std::vector<RenderedObject>& getRenderedObjects() const {
		return m_renderedObjects;
	}

	const ChunkID getId() const {
		return std::make_tuple(_origin.x, _origin.y, _extent);
	}

	void transformHeightData();
	void transformBuildings();
private:
	const Vec2Uint32 _origin;
	const std::uint16_t _extent;

	// Available after .load()
	std::vector<float> heightData;
	std::vector<std::uint32_t> sateliteImage;
	BuildingsLoaderResult buildings;
	std::vector<RenderedObject> m_renderedObjects;
};