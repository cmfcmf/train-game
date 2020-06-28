#pragma once

#include <string>
#include <vector>

#include "util.hpp"

class CityGMLReader
{
public:
	CityGMLReader(const std::string filename) : _filename{filename} {}
	std::pair<std::vector<Vertex>, std::vector<uint32_t>> read() const;

private:
	const std::string _filename;
};