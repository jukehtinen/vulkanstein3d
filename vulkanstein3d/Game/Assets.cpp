#include "../Common.h"

#include "Assets.h"
#include "MeshGenerator.h"

#include "../Rendering/Buffer.h"
#include "../Rendering/Texture.h"

#include "../Wolf3dLoaders/Loaders.h"

#include "xbrz/xbrz.h"

namespace Game
{
std::shared_ptr<Rendering::Buffer> GetScaledTextureArrayData(std::shared_ptr<Rendering::Device> device, Wolf3dLoaders::Bitmap& bitmap, int scaleFactor)
{
    size_t textureSize = bitmap.width * bitmap.height * 4;
    size_t scaledTextureSize = (scaleFactor * bitmap.width) * (scaleFactor * bitmap.height) * 4;

    auto stagingBuffer = Rendering::Buffer::CreateStagingBuffer(device, nullptr, scaledTextureSize * bitmap.layers);
    uint8_t* mapped = (uint8_t*)stagingBuffer->Map();

    for (int i = 0; i < bitmap.layers; i++)
    {
        auto srcptr = reinterpret_cast<uint32_t*>(bitmap.data.data() + (i * textureSize));
        auto dstptr = reinterpret_cast<uint32_t*>(mapped + (i * scaledTextureSize));
        xbrz::scale(scaleFactor, srcptr, dstptr, bitmap.width, bitmap.height, xbrz::ColorFormat::ARGB_UNBUFFERED);
    }

    stagingBuffer->UnMap();
    return stagingBuffer;
}

std::shared_ptr<Rendering::Texture> GetScaledTexture(std::shared_ptr<Rendering::Device> device, Wolf3dLoaders::Loaders& loaders, const std::vector<int> pictures, int scaleFactor)
{
    size_t textureSize = 0;
    size_t scaledTextureSize = 0;
    std::shared_ptr<Rendering::Buffer> stagingBuffer;
    uint8_t* mapped = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;

    for (int i = 0; i < pictures.size(); i++)
    {
        auto bitmap = loaders.LoadPictureTexture(pictures[i]);
        if (textureSize == 0)
        {
            textureSize = bitmap.width * bitmap.height * 4;
            scaledTextureSize = (scaleFactor * bitmap.width) * (scaleFactor * bitmap.height) * 4;
            stagingBuffer = Rendering::Buffer::CreateStagingBuffer(device, nullptr, scaledTextureSize * pictures.size());
            mapped = (uint8_t*)stagingBuffer->Map();
            width = bitmap.width;
            height = bitmap.height;
        }
        assert(bitmap.width == width && bitmap.height == height);

        auto srcptr = reinterpret_cast<uint32_t*>(bitmap.data.data());
        auto dstptr = reinterpret_cast<uint32_t*>(mapped + (i * scaledTextureSize));
        xbrz::scale(scaleFactor, srcptr, dstptr, bitmap.width, bitmap.height, xbrz::ColorFormat::ARGB_UNBUFFERED);
    }

    stagingBuffer->UnMap();
    return Rendering::Texture::CreateTexture(device, stagingBuffer, scaleFactor * width, scaleFactor * height, pictures.size());
}

Assets::Assets(std::shared_ptr<Rendering::Device> device, const std::filesystem::path& dataPath)
{
    Wolf3dLoaders::Loaders loaders{dataPath};

    int scaleFactor = 4;

    AddTexture("tex_gui_loading", GetScaledTexture(device, loaders, {24, 25}, scaleFactor));
    AddTexture("tex_gui_intro", GetScaledTexture(device, loaders, {87}, scaleFactor));
    AddTexture("tex_gui_weapons", GetScaledTexture(device, loaders, {91, 92, 93, 94}, scaleFactor));
    AddTexture("tex_gui_keys", GetScaledTexture(device, loaders, {95, 96, 97}, scaleFactor));

    // numbers 45-
    // letters 56-81
    // numbers white 99-

    auto wallBitmap = loaders.LoadWallTextures();
    auto buffer = GetScaledTextureArrayData(device, wallBitmap, scaleFactor);
    AddTexture("tex_walls", Rendering::Texture::CreateTexture(device, buffer, scaleFactor * wallBitmap.width, scaleFactor * wallBitmap.height, wallBitmap.layers));

    auto spriteBitmap = loaders.LoadSpriteTextures();
    buffer = GetScaledTextureArrayData(device, spriteBitmap, scaleFactor);
    AddTexture("tex_sprites", Rendering::Texture::CreateTexture(device, buffer, scaleFactor * spriteBitmap.width, scaleFactor * spriteBitmap.height, spriteBitmap.layers));
}

Assets::~Assets()
{
}

std::shared_ptr<Rendering::Texture> Assets::GetTexture(const std::string& name)
{
    return _textures[name];
}

void Assets::AddTexture(const std::string& name, std::shared_ptr<Rendering::Texture> texture)
{
    _textures[name] = texture;
}

std::shared_ptr<Rendering::Material> Assets::GetMaterial(const std::string& name)
{
    return _materials[name];
}

void Assets::AddMaterial(const std::string& name, std::shared_ptr<Rendering::Material> texture)
{
    _materials[name] = texture;
}
} // namespace Game