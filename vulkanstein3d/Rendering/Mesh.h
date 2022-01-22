#pragma once

namespace Rendering
{
class Buffer;

class Mesh
{
  public:
    Mesh()
    {
    }

    Mesh(std::shared_ptr<Rendering::Buffer> vertexBuffer, std::shared_ptr<Rendering::Buffer> indexBuffer, uint32_t indexCount)
        : _vertexBuffer(vertexBuffer), _indexBuffer(indexBuffer), _indexCount(indexCount)
    {
    }

    std::shared_ptr<Rendering::Buffer> _vertexBuffer;
    std::shared_ptr<Rendering::Buffer> _indexBuffer;
    uint32_t _indexCount{};
};
} // namespace Rendering