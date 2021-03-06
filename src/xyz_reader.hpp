#pragma once

#include <string>
#include <vector>
#include <tuple>

class XYZReader
{
public:
	XYZReader(const std::string filename) : filename{filename} {}
	std::vector<std::tuple<float, float, float>> read() const;

private:
	const std::string filename;
};