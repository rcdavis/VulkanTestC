#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <string>

namespace FileUtils
{
    std::vector<char> ReadFile(const std::filesystem::path& filepath);

    std::vector<std::string> ReadLines(const std::filesystem::path& filepath);
}
