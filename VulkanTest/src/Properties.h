#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <optional>

class Properties
{
public:
	static Properties ReadFile(const std::filesystem::path& filepath);

public:
	Properties();

	std::optional<std::string> GetString(const std::string& key) const;
	std::optional<uint32_t> GetUInt32(const std::string& key) const;

private:
	std::map<std::string, std::string> mProperties;
};
