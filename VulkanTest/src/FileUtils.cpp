
#include "FileUtils.h"

#include "StringUtils.h"
#include <fstream>
#include <iostream>

namespace FileUtils
{
    std::vector<char> ReadFile(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file " + filepath.string());

        const size_t fileSize = (size_t)std::filesystem::file_size(filepath);
        std::vector<char> buffer(fileSize);

        file.read(std::data(buffer), fileSize);
        file.close();

        return buffer;
    }

    std::vector<std::string> ReadLines(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::in);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file " + filepath.string());

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line))
            lines.push_back(line);

        file.close();
        return lines;
    }
}
