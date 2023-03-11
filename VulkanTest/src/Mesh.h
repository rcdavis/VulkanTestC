#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Vertex.h"

struct aiMesh;

class Mesh
{
public:
    Mesh() = default;

    const std::string& GetName() const { return mName; }
    const std::vector<Vertex>& GetVertices() const { return mVertices; }
    const std::vector<uint16_t>& GetIndices() const { return mIndices; }

    const std::string& GetDiffuseTextureName() const { return mDiffuseTextureName; }

    static std::unique_ptr<Mesh> Load(const aiMesh* const mesh);

private:
    std::string mName;
    std::vector<Vertex> mVertices;
    std::vector<uint16_t> mIndices;

    std::string mDiffuseTextureName;
};
