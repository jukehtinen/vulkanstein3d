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

    const size_t textureSize = 64 * 64 * 4;
    const size_t scaledTextureSize = (scale * 64) * (scale * 64) * 4;

    auto wallBitmap = loaders.LoadWallTextures();
    std::vector<uint8_t> scaledWallTextureArray(scaledTextureSize * wallBitmap.layers);
    for (int i = 0; i < wallBitmap.layers; i++)
    {
        uint32_t* srcptr = reinterpret_cast<uint32_t*>(wallBitmap.data.data() + (i * textureSize));
        uint32_t* dstptr = reinterpret_cast<uint32_t*>(scaledWallTextureArray.data() + (i * scaledTextureSize));

        xbrz::scale(scale, srcptr, dstptr, wallBitmap.width, wallBitmap.height, xbrz::ColorFormat::ARGB);
    }

    auto texture = Rendering::Texture::CreateTexture(device, scaledWallTextureArray.data(), scale * 64, scale * 64, wallBitmap.layers);
    _textures.push_back(texture);

    auto spriteBitmap = loaders.LoadSpriteTextures();
    std::vector<uint8_t> scaledSpriteTextureArray(scaledTextureSize * spriteBitmap.layers);
    for (int i = 0; i < spriteBitmap.layers; i++)
    {
        uint32_t* srcptr = reinterpret_cast<uint32_t*>(spriteBitmap.data.data() + (i * textureSize));
        uint32_t* dstptr = reinterpret_cast<uint32_t*>(scaledSpriteTextureArray.data() + (i * scaledTextureSize));

        xbrz::scale(scale, srcptr, dstptr, spriteBitmap.width, spriteBitmap.height, xbrz::ColorFormat::ARGB);
    }

    texture = Rendering::Texture::CreateTexture(device, scaledSpriteTextureArray.data(), scale * 64, scale * 64, spriteBitmap.layers);
    _textures.push_back(texture);
}

Assets::~Assets()
{
}
} // namespace Game