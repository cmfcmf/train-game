#include <tuple>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

#include "main.hpp"
#include "game.hpp"
#include "xyz_reader.hpp"

void adjustNormal(std::vector<Vertex> &vertices, size_t a, size_t b, size_t c) {
	auto &A = vertices[a];
	auto &B = vertices[b];
	auto &C = vertices[c];

	glm::vec3 normal = glm::normalize(glm::cross(B.pos - A.pos, C.pos - A.pos));
	// normal *= -1;
	// std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

	A.normal += normal;
	B.normal += normal;
	C.normal += normal;
}

float randomBetween0And1() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int main()
{
	const auto reader = XYZReader("dataset/dgm/raw/dgm_33372-5822.xyz");

	const size_t NUM_ROWS = 2001;

	const auto result = reader.read(NUM_ROWS);

	std::cout << "Read " << result.size() << " lines." << std::endl;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto &each : result)
	{
		Vertex vertex = {
			{ -std::get<0>(each), -std::get<1>(each), std::get<2>(each) },
			// { randomBetween0And1(), randomBetween0And1(), randomBetween0And1() },
			{ 0.373f, 0.608f, 0.106f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		};
		const auto color = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		vertex.color *= 1.0f - 0.05f + (0.10 * color);
		vertices.push_back(vertex);
	}


	const auto [min, max] = std::minmax_element(vertices.begin(), vertices.end(), [](const auto &a, const auto &b) {
		return a.pos.z < b.pos.z;
	});

	std::cout << "min: " << min->pos.z << " max: " << max->pos.z << std::endl;

	std::cout << "Generated vertices" << std::endl;

	for (size_t row = 0; row < NUM_ROWS - 1; row++) {
		for (size_t column = 0; column < 2000; column++) {
			const auto tl = column + row * 2001;
			const auto tr = tl + 1;
			const auto bl = column + (row + 1) * 2001;
			const auto br = bl + 1;

			indices.push_back(tr);
			indices.push_back(bl);
			indices.push_back(tl);
			adjustNormal(vertices, tr, bl, tl);

			indices.push_back(tr);
			indices.push_back(br);
			indices.push_back(bl);
			adjustNormal(vertices, tr, br, bl);

			vertices[tl].texCoord = {
				column / 2001.0f,
				row / -2001.0f
			};
		}
	}

	std::cout << "Generated indices" << std::endl;

	std::cout << indices[0] << std::endl << indices[1] << std::endl << indices[2] << std::endl;
	std::cout << vertices[2001].pos.x  << std::endl << vertices[2001].pos.y  << std::endl;


	vertices[0].color = {1.0f, 0.0f, 0.0f};
	vertices[vertices.size() - 1].color = {0.0f, 0.0f, 1.0f};

/*
	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

		{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};

	const std::vector<uint32_t> indices = {
		0, 1, 2,
		2, 3, 0,
		1, 4, 2
	};
*/

	TrainGameApplication app(vertices, indices);

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}