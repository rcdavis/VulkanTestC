#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <memory>

#include "Mesh.h"

class Model
{
public:
    Model() = default;

    const std::string& GetName() const { return mName; }
    const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return mMeshs; }

    const std::string& GetDiffuseTextureName() const { return mDiffuseTextureName; }

    static std::unique_ptr<Model> Load(const std::filesystem::path& filepath);

private:
    std::string mName;
    std::vector<std::unique_ptr<Mesh>> mMeshs;

    std::string mDiffuseTextureName;
};
