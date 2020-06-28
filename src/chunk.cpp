#include "chunk.hpp"
#include <cassert>


void Chunk::load(HeightDataLoader &heightDataLoader, SateliteImageLoader &sateliteImageLoader, BuildingsLoader &buildingsLoader) {
	heightData = heightDataLoader.load(_origin.x, _origin.y, _extent);
	sateliteImage = sateliteImageLoader.load(_origin.x, _origin.y, _extent);
	buildings = buildingsLoader.load(_origin.x, _origin.y, _extent);
}
