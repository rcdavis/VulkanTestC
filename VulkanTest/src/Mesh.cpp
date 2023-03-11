#include "Mesh.h"

#include "Vertex.h"
#include "Log.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

std::unique_ptr<Mesh> Mesh::Load(const std::filesystem::path& filepath)
{
    constexpr uint32_t flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices;

    Assimp::Importer importer;
    const aiScene* const scene = importer.ReadFile(filepath.string(), flags);
    if (!scene)
    {
        LOG_ERROR("Failed to load mesh {0}: {1}", filepath.string(), importer.GetErrorString());
        return {};
    }

    auto loadedMesh = std::make_unique<Mesh>();
    //loadedMesh->mName = scene->mName.C_Str();

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* const mesh = scene->mMeshes[i];
        loadedMesh->mName = mesh->mName.C_Str();
        for (uint32_t meshI = 0; meshI < mesh->mNumVertices; ++meshI)
        {
            const aiVector3D pos = mesh->mVertices[meshI];

            aiVector3D normal(0.0f, 0.0f, 0.0f);
            if (mesh->HasNormals())
                normal = mesh->mNormals[meshI];

            aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
            if (mesh->HasVertexColors(0))
                color = mesh->mColors[0][meshI];

            aiVector3D texCoord(0.0f, 0.0f, 0.0f);
            if (mesh->HasTextureCoords(0))
                texCoord = mesh->mTextureCoords[0][meshI];

            Vertex vert;
            vert.pos = { pos.x, pos.y, pos.z };
            vert.normal = { normal.x, normal.y, normal.z };
            vert.color = { color.r, color.g, color.b };
            vert.texCoord = { texCoord.x, texCoord.y };
            loadedMesh->mVertices.push_back(vert);
        }

        for (uint32_t index = 0; index < mesh->mNumFaces; ++index)
        {
            const aiFace& face = mesh->mFaces[index];
            for (uint32_t faceI = 0; faceI < face.mNumIndices; ++faceI)
                loadedMesh->mIndices.push_back(face.mIndices[faceI]);
        }
    }

    for (size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* const material = scene->mMaterials[i];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
            {
                loadedMesh->mDiffuseTextureName = "assets/textures/" + std::filesystem::path(path.C_Str()).filename().string();
            }
        }
    }

    return loadedMesh;
}
