#include "xyz_reader.hpp"
#include <fstream>

std::vector<std::tuple<float, float, float>> XYZReader::read() const
{
	std::vector<std::tuple<float, float, float>> result;

	std::ifstream infile(filename);
	float x, y, z;
	while (infile >> x >> y >> z)
	{
		result.emplace_back(x, y, z);
	}

	return result;
}