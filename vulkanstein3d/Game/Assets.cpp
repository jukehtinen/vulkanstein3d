#include "../Common.h"

#include "Assets.h"
#include "MeshGenerator.h"

#include "../Rendering/Texture.h"
#include "../Wolf3dLoaders/Loaders.h"

#include "xbrz/xbrz.h"

namespace Game
{
Assets::Assets(std::shared_ptr<Rendering::Device> device, const std::filesystem::path& dataPath)
{
    Wolf3dLoaders::Loaders loaders{dataPath};

    auto introBitmap = loaders.LoadPictureTexture(Wolf3dLoaders::Pictures::IntroScreen);

    int scale = 4;
    if (scale == 1)
    {
        auto texture = Rendering::Texture::CreateTexture(device, introBitmap.data.data(), introBitmap.width, introBitmap.height);
        _textures.push_back(texture);
    }
    else
    {
        std::vector<uint8_t> scaledData;
        scaledData.resize((scale * introBitmap.width) * (scale * introBitmap.height) * 4);
        xbrz::scale(scale, (uint32_t*)introBitmap.data.data(), (uint32_t*)scaledData.data(), introBitmap.width, introBitmap.height, xbrz::ColorFormat::ARGB);

        auto texture = Rendering::Texture::CreateTexture(device, scaledData.data(), scale * introBitmap.width, scale * introBitmap.height);
        _textures.push_back(texture);
    }

    size_t textureSize = (scale * 64) * (scale * 64) * 4;
    std::vector<uint8_t> wallTextureArray(textureSize * 100);
    for (int i = 0; i < 100; i++)
    {
        auto wallBitmap = loaders.LoadWallTexture(i);

        uint32_t* ptr = reinterpret_cast<uint32_t*>(wallTextureArray.data() + (i * textureSize));
        xbrz::scale(scale, (uint32_t*)wallBitmap.data.data(), ptr, wallBitmap.width, wallBitmap.height, xbrz::ColorFormat::ARGB);
    }

    auto texture = Rendering::Texture::CreateTexture(device, wallTextureArray.data(), scale * 64, scale * 64, 100);
    _textures.push_back(texture);
}

Assets::~Assets()
{
}
} // namespace Game