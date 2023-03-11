#include "Mesh.h"

#include <assimp/mesh.h>

std::unique_ptr<Mesh> Mesh::Load(const aiMesh* const mesh)
{
    auto loadedMesh = std::make_unique<Mesh>();
    loadedMesh->mName = mesh->mName.C_Str();

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D pos = mesh->mVertices[i];

        aiVector3D normal(0.0f, 0.0f, 0.0f);
        if (mesh->HasNormals())
            normal = mesh->mNormals[i];

        aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
        if (mesh->HasVertexColors(0))
            color = mesh->mColors[0][i];

        aiVector3D texCoord(0.0f, 0.0f, 0.0f);
        if (mesh->HasTextureCoords(0))
            texCoord = mesh->mTextureCoords[0][i];

        Vertex vert;
        vert.pos = { pos.x, pos.y, pos.z };
        vert.normal = { normal.x, normal.y, normal.z };
        vert.color = { color.r, color.g, color.b };
        vert.texCoord = { texCoord.x, texCoord.y };
        loadedMesh->mVertices.push_back(vert);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for (uint32_t faceI = 0; faceI < face.mNumIndices; ++faceI)
            loadedMesh->mIndices.push_back(face.mIndices[faceI]);
    }

    return loadedMesh;
}
