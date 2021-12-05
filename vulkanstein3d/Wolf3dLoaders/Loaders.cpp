#include "Loaders.h"
#include "Palette.h"

#include "spdlog/spdlog.h"

#include <array>
#include <fstream>

// https://github.com/id-Software/wolf3d/blob/master/WOLFSRC
namespace Wolf3dLoaders
{

constexpr int MaxMapPlanes = 3;
constexpr int EpisodeLevels = 10;
constexpr int MaxLevels = 100;

struct Chunks
{
    uint16_t chunks;
    uint16_t spriteStart;
    uint16_t soundStart;
    std::vector<uint32_t> offsets;
    std::vector<uint16_t> lengths;
};

#pragma pack(2)
struct HuffmanNode
{
    int16_t node0;
    int16_t node1;
};

#pragma pack(2)
struct LevelHeader
{
    int32_t planeOffset[MaxMapPlanes];
    uint16_t planeCompressedLength[MaxMapPlanes];
    uint16_t width;
    uint16_t height;
    uint8_t name[16];
};

#pragma pack(2)
struct MapHeader
{
    uint16_t rlewMagic;
    int32_t levelPointers[MaxLevels];
};

struct Size
{
    int16_t width;
    int16_t height;
};

void CarmackExpand(uint8_t* source, uint16_t* dest, int length)
{
    const uint16_t NEARTAG = 0xa7;
    const uint16_t FARTAG = 0xa8;

    length /= 2;

    auto inptr = (uint8_t*)source;
    auto outptr = dest;

    while (length > 0)
    {
        auto ch = inptr[0] | inptr[1] << 8;
        inptr += 2;
        auto chhigh = ch >> 8;
        if (chhigh == NEARTAG)
        {
            auto count = ch & 0xff;
            if (!count)
            {
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            }
            else
            {
                auto offset = *inptr++;
                auto copyptr = outptr - offset;
                length -= count;
                if (length < 0)
                    return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else if (chhigh == FARTAG)
        {
            auto count = ch & 0xff;
            if (!count)
            {
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            }
            else
            {
                auto offset = inptr[0] | inptr[1] << 8;
                inptr += 2;
                auto copyptr = dest + offset;
                length -= count;
                if (length < 0)
                    return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else
        {
            *outptr++ = ch;
            length--;
        }
    }
}

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

void RLEWexpand(uint16_t* source, uint16_t* dest, int32_t length, uint16_t rlewtag)
{
    uint16_t value, count, i;
    uint16_t* end = dest + length / 2;

    do
    {
        value = *source++;
        if (value != rlewtag)
        {
            *dest++ = value;
        }
        else
        {
            count = *source++;
            value = *source++;
            for (i = 1; i <= count; i++)
                *dest++ = value;
        }
    } while (dest < end);
}

Chunks LoadChunks(std::ifstream& file)
{
    Chunks chunks{};
    file.read(reinterpret_cast<char*>(&chunks.chunks), sizeof(uint16_t));
    file.read(reinterpret_cast<char*>(&chunks.spriteStart), sizeof(uint16_t));
    file.read(reinterpret_cast<char*>(&chunks.soundStart), sizeof(uint16_t));

    chunks.offsets.resize(chunks.chunks);
    chunks.lengths.resize(chunks.chunks);

    file.read(reinterpret_cast<char*>(chunks.offsets.data()), sizeof(uint32_t) * chunks.chunks);
    file.read(reinterpret_cast<char*>(chunks.lengths.data()), sizeof(uint16_t) * chunks.chunks);

    return chunks;
}

Loaders::Loaders(const std::filesystem::path& dataPath)
    : _dataPath(dataPath)
{
}

Bitmap Loaders::LoadPictureTexture(Pictures picture)
{
    spdlog::info("[Wolf3dLoaders] Loading picture {}", picture);

    std::ifstream dictFile((_dataPath / "VGADICT.WL6"), std::ios::binary);

    std::vector<HuffmanNode> huffmanTree(255);
    dictFile.read(reinterpret_cast<char*>(huffmanTree.data()), sizeof(HuffmanNode) * huffmanTree.size());

    std::ifstream headFile((_dataPath / "VGAHEAD.WL6"), std::ios::binary);
    std::vector<int32_t> offsets(149);
    unsigned char bytes[3] = {};
    for (int i = 0; i < 149; i++)
    {
        headFile.read(reinterpret_cast<char*>(bytes), sizeof(unsigned char) * 3);

        offsets[i] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16);
        if (offsets[i] == 0x00FFFFFF)
            offsets[i] = -1;
    }

    std::ifstream graphFile((_dataPath / "VGAGRAPH.WL6"), std::ios::binary);
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

Bitmap Loaders::LoadWallTexture(int index)
{
    spdlog::info("[Wolf3dLoaders] Loading wall texture {}", index);

    std::ifstream file((_dataPath / "VSWAP.WL6"), std::ios::binary);

    const auto chunks = Wolf3dLoaders::LoadChunks(file);

    Bitmap bitmap;
    bitmap.width = bitmap.height = 64;
    bitmap.data.resize(bitmap.width * bitmap.height * 4);
    // bitmap.layers = chunks.spriteStart;
    // bitmap.data.resize(64 * 64 * 4 * chunks.spriteStart);

    std::vector<uint8_t> buffer(chunks.lengths[0]);
    //for (int i = 0; i < chunks.spriteStart; i++)
    {
        file.seekg(chunks.offsets[index], std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), chunks.lengths[index]);

        for (int y = 0; y < 64; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                const auto& color = Palette[buffer[(x * 64 + y)]];
                std::memcpy(bitmap.data.data() + (((y * 64 + x)) * 4), &color, 4);
            }
        }
    }

    return bitmap;
}

Map Loaders::LoadMap(int episode, int level)
{
    spdlog::info("[Wolf3dLoaders] Loading episode {} level {}", episode, level);

    std::ifstream headerFile((_dataPath / "MAPHEAD.WL6"), std::ios::binary);
    std::ifstream mapFile((_dataPath / "GAMEMAPS.WL6"), std::ios::binary);

    if (!headerFile.is_open() || !mapFile.is_open())
    {
        spdlog::error("[Wolf3dLoaders] Couldn't open datafile");
        return {};
    }

    MapHeader mapHeader{};
    headerFile.read(reinterpret_cast<char*>(&mapHeader), sizeof(MapHeader));

    const int levelIndex = (episode - 1) * EpisodeLevels + level - 1;
    if (mapHeader.levelPointers[levelIndex] == 0)
    {
        spdlog::error("[Wolf3dLoaders] Level not found");
        return {};
    }

    LevelHeader levelHeader{};
    mapFile.seekg(mapHeader.levelPointers[levelIndex], std::ios::beg);
    mapFile.read(reinterpret_cast<char*>(&levelHeader), sizeof(LevelHeader));

    const auto mapSize = levelHeader.width * levelHeader.width * 2;

    Map map;
    map.width = levelHeader.width;
    for (auto plane = 0; plane < 2; plane++)
    {
        std::vector<uint8_t> carmackBuffer(levelHeader.planeCompressedLength[plane]);
        mapFile.seekg(levelHeader.planeOffset[plane], std::ios::beg);
        mapFile.read(reinterpret_cast<char*>(carmackBuffer.data()), sizeof(uint8_t) * levelHeader.planeCompressedLength[plane]);

        auto source = reinterpret_cast<uint16_t*>(carmackBuffer.data());
        auto expandedSize = *source;
        source++;

        std::vector<uint8_t> expandBuffer(expandedSize);
        CarmackExpand(reinterpret_cast<uint8_t*>(source), reinterpret_cast<uint16_t*>(expandBuffer.data()), expandedSize);

        map.tiles[plane].resize(mapSize);
        RLEWexpand((reinterpret_cast<uint16_t*>(expandBuffer.data())) + 1, reinterpret_cast<uint16_t*>(map.tiles[plane].data()), mapSize, mapHeader.rlewMagic);
    }

    // Wolf has two images per tile (light and dark). Use only light version.
    for (int i = 0; i < map.tiles[0].size(); i++)
    {
        auto tileindex = map.tiles[0][i];
        tileindex--;
        tileindex *= 2;
        if (tileindex % 2 != 0)
            tileindex++;
        map.tiles[0][i] = tileindex;
    }

    for (int i = 0; i < map.tiles[1].size(); i++)
    {
        auto tileindex = map.tiles[1][i];
        if (tileindex == 19 || tileindex == 20 || tileindex == 21 || tileindex == 22)
            map.playerStart = i;
    }

    return map;
}

} // namespace Wolf3dLoaders