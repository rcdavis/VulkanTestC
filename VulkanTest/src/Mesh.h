#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <memory>

struct Vertex;

class Mesh
{
public:
    Mesh() = default;

    const std::string& GetName() const { return mName; }
    const std::vector<Vertex>& GetVertices() const { return mVertices; }
    const std::vector<uint16_t>& GetIndices() const { return mIndices; }

    static std::unique_ptr<Mesh> Load(const std::filesystem::path& filepath);

private:
    std::string mName;
    std::vector<Vertex> mVertices;
    std::vector<uint16_t> mIndices;
};
