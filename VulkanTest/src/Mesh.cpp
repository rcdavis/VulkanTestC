#include "Mesh.h"

#include "Vertex.h"
#include "Log.h"

#include <tiny_obj_loader.h>

std::unique_ptr<Mesh> Mesh::Load(const std::filesystem::path& filepath)
{
    auto loadedMesh = std::make_unique<Mesh>();

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filepath.string()))
    {
        if (!reader.Error().empty())
        {
            LOG_ERROR("TinyObjReader error loading {0}: {1}", filepath.string(), reader.Error());
            return {};
        }
    }

    if (!reader.Warning().empty())
        LOG_WARN("TinyObj: {0}", reader.Warning());

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    std::unordered_map<Vertex, uint32_t> uniqueVerts;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vert;

            vert.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (!std::empty(attrib.colors))
            {
                vert.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }

            if (index.texcoord_index >= 0)
            {
                vert.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (uniqueVerts.count(vert) == 0)
            {
                uniqueVerts[vert] = (uint32_t)std::size(loadedMesh->mVertices);
                loadedMesh->mVertices.push_back(vert);
            }

            loadedMesh->mIndices.push_back(uniqueVerts[vert]);
        }
    }

    return loadedMesh;
}
