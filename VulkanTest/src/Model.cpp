#include "Model.h"

#include "Log.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

std::unique_ptr<Model> Model::Load(const std::filesystem::path& filepath)
{
    constexpr uint32_t flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices;

    Assimp::Importer importer;
    const aiScene* const scene = importer.ReadFile(filepath.string(), flags);
    if (!scene)
    {
        LOG_ERROR("Failed to load mesh {0}: {1}", filepath.string(), importer.GetErrorString());
        return {};
    }

    auto loadedModel = std::make_unique<Model>();
    loadedModel->mName = scene->mName.C_Str();

    loadedModel->mMeshs.reserve(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        loadedModel->mMeshs.emplace_back(Mesh::Load(scene->mMeshes[i]));
    }

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* const material = scene->mMaterials[i];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
            {
                loadedModel->mDiffuseTextureName = "assets/textures/" + std::filesystem::path(path.C_Str()).filename().string();
            }
        }
    }

    return loadedModel;
}
