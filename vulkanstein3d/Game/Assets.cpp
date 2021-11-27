#include "Assets.h"

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
}

Assets::~Assets()
{
}
} // namespace Game