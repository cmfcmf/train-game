#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <string>
#include <filesystem>
#include <regex>
#include <iostream>
#include <cstdlib>

#include <spdlog/spdlog.h>

#include "../exception.hpp"

template<typename T, typename F>
class Loader
{
public:
	T load(std::uint32_t requestedX, std::uint32_t requestedY, std::uint16_t extent) {
		const auto chunkOrigins = getChunkOrigins(requestedX, requestedY, extent);
		T result = initResult(requestedX, requestedY, extent);

		for (const auto &chunkOrigin : chunkOrigins) {
			const auto chunkId = std::to_string(UTM_ZONE) + std::to_string(chunkOrigin.first / 1000) + "-" + std::to_string(chunkOrigin.second / 1000);

			const auto zipName = std::regex_replace(getZipName(), std::regex("\\{\\}"), chunkId);
			const std::filesystem::path zipPath = "datasets/" + getName() + "/zips/" + zipName;
			if (!std::filesystem::exists(zipPath)) {
				spdlog::warn("File {} needs to be downloaded.", zipName);
				const auto success = download(getDownloadUrl() + zipName, zipPath);
				if (!success) {
					// No data available
					continue;
				}
			}

			const auto rawName = std::regex_replace(getRawName(), std::regex("\\{\\}"), chunkId);
			const std::filesystem::path rawPath = "datasets/" + getName() + "/raw/" + rawName;
			if (!std::filesystem::exists(rawPath)) {
				spdlog::warn("File {} needs to be unzipped.", rawName);
				unzip(zipPath, rawPath.parent_path(), rawName);
			}

			const auto filename = rawPath;
			if (!_cache.contains(filename)) {
				spdlog::debug("Loading and parsing file {}.", filename.string());
				_cache.insert({filename, parseFile(filename)});
			}
			const F rawData = _cache.at(filename);

			loadChunk(result, rawData, chunkOrigin.first, chunkOrigin.second, requestedX, requestedY, extent);
		}

		return result;
	};
protected:
	static const auto UTM_ZONE = 33;

	std::map<const std::string, const F> _cache;

	virtual std::uint32_t getChunkSize() const = 0;
	virtual std::string getName() const = 0;
	virtual std::string getZipName() const = 0;
	virtual std::string getRawName() const = 0;
	virtual std::string getDownloadUrl() const = 0;

	virtual T initResult(const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const = 0;
	virtual F parseFile(const std::string &filename) const = 0;
	virtual void loadChunk(T&, const F &rawData, const std::uint32_t originX, const std::uint32_t originY,
		const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const = 0;

	std::vector<std::pair<std::uint32_t, std::uint32_t>> getChunkOrigins(const std::uint32_t requestedX, const std::uint32_t requestedY, const std::uint16_t extent) const {
		const std::uint32_t baseX = requestedX - (requestedX % getChunkSize());
		const std::uint32_t baseY = requestedY - (requestedY % getChunkSize());

		std::vector<std::pair<std::uint32_t, std::uint32_t>> origins;
		for (auto x = baseX; x < requestedX + extent; x += getChunkSize()) {
			for (auto y = baseY; y < requestedY + extent; y += getChunkSize()) {
				origins.emplace_back(x, y);
			}
		}

		assert(origins.size() > 0);
		return origins;
	}

	void unzip(std::filesystem::path archive, std::filesystem::path destination, std::string filename) {
		const auto code = execute("unzip -o '" + archive.string() + + "' '" + filename + "' -d '" + destination.string() + "'");
		if (code != 0 && code != 1) {
			// exit code 1 still indicates successful extraction.
			throw_with_trace(std::runtime_error("Command exited with a non-zero status code: " + std::to_string(code)));
		}
	}

	bool download(std::string url, std::filesystem::path destination) {
		return execute("curl --silent --show-error --fail -o '" + destination.string() + "' " + url);
	}

	bool execute(std::string command) {
		spdlog::debug("Executing command: {}", command);
		return 0 == std::system(command.c_str());
	}
};