
#include "Properties.h"

#include "FileUtils.h"
#include <iostream>

Properties Properties::ReadFile(const std::filesystem::path& filepath)
{
    const auto lines = FileUtils::ReadLines(filepath);
    Properties props;
    for (const auto& line : lines)
    {
        const auto firstIndex = line.find_first_of("=");
        if (firstIndex == std::string::npos)
        {
            std::cout << "Failed to find \"=\" in property entry line: " << line << std::endl;
            continue;
        }

        const auto key = line.substr(0, firstIndex);
        if (std::empty(key))
        {
            std::cout << "Failed to find key in property entry line: " << line << std::endl;
            continue;
        }

        const auto val = line.substr(firstIndex + 1);
        if (std::empty(val))
        {
            std::cout << "Failed to find value in property entry line: " << line << std::endl;
            continue;
        }

        props.mProperties.insert(std::pair(key, val));
    }
    return props;
}

Properties::Properties() :
	mProperties()
{}

std::optional<std::string> Properties::GetString(const std::string& key) const
{
	const auto iter = mProperties.find(key);
	if (iter == std::end(mProperties))
		return {};

	return iter->second;
}

std::optional<uint32_t> Properties::GetUInt32(const std::string& key) const
{
	const auto iter = mProperties.find(key);
	if (iter == std::end(mProperties))
		return {};

	return (uint32_t)std::stoi(iter->second);
}
