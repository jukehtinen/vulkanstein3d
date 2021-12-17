#include "../Common.h"

#include "../Rendering/Buffer.h"

#include "MeshGenerator.h"

namespace Game
{
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 uvTile;
};

static void GenerateCube(Vertex* verts, uint32_t* indices, uint32_t tileId)
{
    const glm::vec3 normal{0.0f, 1.0f, 0.0f};

    // Front
    verts[0] = {glm::vec3{-0.5f, -0.5f, 0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[1] = {glm::vec3{0.5f, -0.5f, 0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[2] = {glm::vec3{0.5f, 0.5f, 0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[3] = {glm::vec3{-0.5f, 0.5f, 0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};
    // Top
    verts[4] = {glm::vec3{-0.5f, 0.5f, 0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[5] = {glm::vec3{0.5f, 0.5f, 0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[6] = {glm::vec3{0.5f, 0.5f, -0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[7] = {glm::vec3{-0.5f, 0.5f, -0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};
    // Back
    verts[8] = {glm::vec3{0.5f, -0.5f, -0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[9] = {glm::vec3{-0.5f, -0.5f, -0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[10] = {glm::vec3{-0.5f, 0.5f, -0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[11] = {glm::vec3{0.5f, 0.5f, -0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};
    // Bottom
    verts[12] = {glm::vec3{-0.5f, -0.5f, -0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[13] = {glm::vec3{0.5f, -0.5f, -0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[14] = {glm::vec3{0.5f, -0.5f, 0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[15] = {glm::vec3{-0.5f, -0.5f, 0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};
    // Left
    verts[16] = {glm::vec3{-0.5f, -0.5f, -0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[17] = {glm::vec3{-0.5f, -0.5f, 0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[18] = {glm::vec3{-0.5f, 0.5f, 0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[19] = {glm::vec3{-0.5f, 0.5f, -0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};
    // Right
    verts[20] = {glm::vec3{0.5f, -0.5f, 0.5f}, normal, glm::vec3{0.0f, 1.0f, tileId}};
    verts[21] = {glm::vec3{0.5f, -0.5f, -0.5f}, normal, glm::vec3{1.0f, 1.0f, tileId}};
    verts[22] = {glm::vec3{0.5f, 0.5f, -0.5f}, normal, glm::vec3{1.0f, 0.0f, tileId}};
    verts[23] = {glm::vec3{0.5f, 0.5f, 0.5f}, normal, glm::vec3{0.0f, 0.0f, tileId}};

    const uint32_t cubeIndices[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
    std::memcpy(&indices[0], &cubeIndices[0], 36 * sizeof(uint32_t));
}

Game::Mesh MeshGenerator::BuildFloorPlaneMesh(std::shared_ptr<Rendering::Device> device, int size)
{
    const glm::vec3 normal{0.0f, 1.0f, 0.0f};
    const uint32_t quadIndices[] = {0, 2, 1, 1, 2, 3};

    std::vector<Vertex> verts(size * size * 4);
    std::vector<uint32_t> indices(size * size * 6);

    for (int i = 0; i < size * size; i++)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        verts[i * 4 + 0] = {glm::vec3{0.0f, 0.0f, 0.0f}, normal, glm::vec3{r, g, b}};
        verts[i * 4 + 1] = {glm::vec3{1.0f, 0.0f, 0.0f}, normal, glm::vec3{r, g, b}};
        verts[i * 4 + 2] = {glm::vec3{0.0f, 0.0f, 1.0f}, normal, glm::vec3{r, g, b}};
        verts[i * 4 + 3] = {glm::vec3{1.0f, 0.0f, 1.0f}, normal, glm::vec3{r, g, b}};

        for (auto v = 0; v < 4; v++)
        {
            verts[i * 4 + v].pos.x += (i % size);
            verts[i * 4 + v].pos.z += (i / size);
        }

        for (auto n = 0; n < 6; n++)
        {
            indices[i * 6 + n] = quadIndices[n] + i * 4;
        }
    }

    auto vertexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, verts.data(), verts.size() * sizeof(Vertex));
    auto indexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, indices.data(), indices.size() * sizeof(uint32_t));

    auto vertexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, verts.size() * sizeof(Vertex));
    auto indexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, indices.size() * sizeof(uint32_t));

    vertexBufferStaging->CopyTo(vertexBuffer);
    indexBufferStaging->CopyTo(indexBuffer);

    return {vertexBuffer, indexBuffer, (uint32_t)indices.size()};
}

Game::Mesh MeshGenerator::BuildCubeMesh(std::shared_ptr<Rendering::Device> device)
{
    std::vector<Vertex> verts(6 * 4);
    std::vector<uint32_t> indices(36);

    GenerateCube(verts.data(), indices.data(), 0);

    auto vertexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, verts.data(), verts.size() * sizeof(Vertex));
    auto indexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, indices.data(), indices.size() * sizeof(uint32_t));

    auto vertexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, verts.size() * sizeof(Vertex));
    auto indexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, indices.size() * sizeof(uint32_t));

    vertexBufferStaging->CopyTo(vertexBuffer);
    indexBufferStaging->CopyTo(indexBuffer);

    return {vertexBuffer, indexBuffer, (uint32_t)indices.size()};
}

Game::Mesh MeshGenerator::BuildMapMesh(std::shared_ptr<Rendering::Device> device, const Wolf3dLoaders::Map& map)
{
    const auto solidWallCount = std::count_if(std::begin(map.tiles[0]), std::end(map.tiles[0]), [](uint16_t i) { return i != 0 && i <= 53; });

    std::vector<Vertex> verts(solidWallCount * 6 * 4);
    std::vector<uint32_t> indices(solidWallCount * 36);

    uint32_t cubeIndex = 0;

    for (int i = 0; i < map.width * map.width; i++)
    {
        int tileId = map.tiles[0][i];

        if (tileId == 0 || tileId > 53)
            continue;

        // Handle secret doors as entities.
        if (map.tiles[1][i] == 98)
            continue;

        // Wolf has two images per tile (light and dark). Use only light version.
        tileId--;
        tileId *= 2;
        if (tileId % 2 != 0)
            tileId++;

        GenerateCube(verts.data() + (cubeIndex * 24), indices.data() + (cubeIndex * 36), tileId);

        for (auto n = 0; n < 36; n++)
        {
            indices[cubeIndex * 36 + n] = indices[cubeIndex * 36 + n] + cubeIndex * 24;
        }

        for (auto v = 0; v < 24; v++)
        {
            verts[cubeIndex * 24 + v].pos.x += (i % map.width);
            verts[cubeIndex * 24 + v].pos.y += 0.5f;
            verts[cubeIndex * 24 + v].pos.z += (i / map.width);
        }

        cubeIndex++;
    }

    auto vertexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, verts.data(), verts.size() * sizeof(Vertex));
    auto indexBufferStaging = Rendering::Buffer::CreateStagingBuffer(device, indices.data(), indices.size() * sizeof(uint32_t));

    auto vertexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, verts.size() * sizeof(Vertex));
    auto indexBuffer = Rendering::Buffer::CreateGPUBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, indices.size() * sizeof(uint32_t));

    vertexBufferStaging->CopyTo(vertexBuffer);
    indexBufferStaging->CopyTo(indexBuffer);

    return {vertexBuffer, indexBuffer, (uint32_t)indices.size()};
}
} // namespace Game