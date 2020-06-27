#include "xyz_reader.hpp"
#include <fstream>

std::vector<std::tuple<float, float, float>> XYZReader::read(const size_t numRows) const
{
	std::vector<std::tuple<float, float, float>> result;

	std::ifstream infile(filename);
	float x, y, z;
	float lastY = -1;
	float offsetX, offsetY;
	for (size_t row = 0; infile >> x >> y >> z;)
	{
		if (row == 0) {
			offsetX = x;
			offsetY = y;
		}

		if (y != lastY) {
			lastY = y;
			row++;
			if (row > numRows) {
				break;
			}
		}
		result.emplace_back(x - offsetX, y - offsetY, z);

	}

	return result;
}