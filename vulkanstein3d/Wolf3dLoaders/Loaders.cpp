#include "Loaders.h"
#include "Palette.h"

#include "spdlog/spdlog.h"

#include <fstream>

// https://github.com/id-Software/wolf3d/blob/master/WOLFSRC
namespace Wolf3dLoaders
{

#pragma pack(2)
struct HuffmanNode
{
    int16_t node0;
    int16_t node1;
};

struct Size
{
    int16_t width;
    int16_t height;
};

int HuffmanExpand(uint8_t* source, uint8_t* destination, int32_t length, HuffmanNode* tree)
{
    uint8_t* read = source;
    uint8_t* write = destination;
    uint8_t mask = 0x01;
    uint8_t input = *(read++);

    int16_t nodeValue;
    HuffmanNode* node = &tree[254];
    int32_t bytesWritten = 0;

    while (1)
    {
        if ((input & mask) == 0)
            nodeValue = node->node0;
        else
            nodeValue = node->node1;

        if (mask == 0x80)
        {
            input = *(read++);
            mask = 0x01;
        }
        else
        {
            mask <<= 1;
        }

        if (nodeValue <= 0xFF)
        {
            *(write++) = (uint8_t)nodeValue;
            node = &tree[254];
            if ((++bytesWritten) == length)
                break;
        }
        else
        {
            node = tree + nodeValue - 256;
        }
    }

    return 0;
}

Loaders::Loaders(const std::filesystem::path& dataPath)
    : _dataPath(dataPath)
{
}

Bitmap Loaders::LoadPictureTexture(Pictures picture)
{
    spdlog::info("[Wolf3dLoaders] Loading picture {}", picture);

    const auto dictFilename = (_dataPath / "VGADICT.WL6");
    const auto headerFilename = (_dataPath / "VGAHEAD.WL6");
    const auto graphFilename = (_dataPath / "VGAGRAPH.WL6");

    std::ifstream dictFile(dictFilename, std::ios::binary);

    std::vector<HuffmanNode> huffmanTree(255);
    dictFile.read(reinterpret_cast<char*>(huffmanTree.data()), sizeof(HuffmanNode) * huffmanTree.size());

    std::ifstream headFile(headerFilename, std::ios::binary);
    std::vector<int32_t> offsets(149);
    unsigned char bytes[3] = {};
    for (int i = 0; i < 149; i++)
    {
        headFile.read(reinterpret_cast<char*>(bytes), sizeof(unsigned char) * 3);

        offsets[i] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16);
        if (offsets[i] == 0x00FFFFFF)
            offsets[i] = -1;
    }

    std::ifstream graphFile(graphFilename, std::ios::binary);
    std::vector<Size> picTable(132);

    const auto compressedLength = offsets[1] - offsets[0] - 4;
    int32_t expandedLength = 0;
    graphFile.read(reinterpret_cast<char*>(&expandedLength), sizeof(int32_t));

    std::vector<uint8_t> compressed(compressedLength);
    std::vector<uint8_t> expanded(expandedLength);

    graphFile.read(reinterpret_cast<char*>(compressed.data()), compressedLength);

    HuffmanExpand(compressed.data(), expanded.data(), expandedLength, huffmanTree.data());

    for (int i = 0; i < picTable.size(); ++i)
    {
        picTable[i].width = expanded[4 * i] | (expanded[4 * i + 1] << 8);
        picTable[i].height = expanded[4 * i + 2] | (expanded[4 * i + 3] << 8);
    }

    uint32_t picId = (uint32_t)picture;
    uint32_t index = picId + 1;
    while (offsets[index] == -1)
        ++index;

    const auto imageCompressedLength = offsets[index] - offsets[picId];
    std::vector<uint8_t> imageCompressed(imageCompressedLength);

    auto compressedChunkPtr = reinterpret_cast<int32_t*>(imageCompressed.data());

    graphFile.seekg(offsets[picId], std::ios::beg);
    graphFile.read(reinterpret_cast<char*>(imageCompressed.data()), sizeof(uint8_t) * imageCompressedLength);
    auto imageExpandedLength = *(compressedChunkPtr++);
    std::vector<uint8_t> imageExpanded(imageExpandedLength);

    HuffmanExpand(reinterpret_cast<uint8_t*>(compressedChunkPtr), imageExpanded.data(), imageExpandedLength, huffmanTree.data());

    Bitmap bitmap;
    bitmap.width = picTable[picId - 3].width;
    bitmap.height = picTable[picId - 3].height;
    bitmap.layers = 1;
    bitmap.data.resize(bitmap.width * bitmap.height * 4);

    for (int y = 0; y < bitmap.height; y++)
    {
        for (int x = 0; x < bitmap.width; x++)
        {
            const auto& color = Palette[imageExpanded[(y * (bitmap.width >> 2) + (x >> 2)) + (x & 3) * (bitmap.width >> 2) * bitmap.height]];
            std::memcpy(bitmap.data.data() + ((y * bitmap.width + x) * 4), &color, 4);
        }
    }

    return bitmap;
}

} // namespace Wolf3dLoaders