#include "Rendering/Texture.h"
#include "Core/Log.h"
#include "vk/VkTexture.h"
#include "vk/VkContext.h"
#include <stb_image/stb_image.h>
#include <bit>

namespace aby {

    Resource Texture::create(Context* ctx, const fs::path& path) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN: {
                auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), path);
                return ctx->textures().add(tex);
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
        }
        return {};
    }

    Resource Texture::create(Context* ctx) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN:
            {
                auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx));
                return ctx->textures().add(tex);
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
        }
        return {};
    }

    Resource Texture::create(Context* ctx, const glm::u32vec2& size, const glm::vec4& color) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN:
            {
                auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), size, color);
                return ctx->textures().add(tex);
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
        }
        return {};
    }


    Texture::Texture() :
        m_Size(0, 0),
        m_Channels(0) { }


    Texture::Texture(const fs::path& path) :
        m_Size(0, 0),
        m_Channels(0)
    {
        auto str = path.string();
        const char* file = str.c_str();
        int w, h, c;
        constexpr int LOAD_ALL_CHANNELS = 0;

        unsigned char* data = stbi_load(file, &w, &h, &c, LOAD_ALL_CHANNELS);
        if (!data) {
            ABY_ERR("[stbi_image::stbi_load]: {}", stbi_failure_reason());
            return;  
        }
        auto ptr   = reinterpret_cast<std::byte*>(data);
        m_Size.x   = static_cast<float>(w);
        m_Size.y   = static_cast<float>(h);
        m_Channels = c;
        m_Data.assign(ptr, ptr + (w * h * c));
        stbi_image_free(data);
    }

    Texture::Texture(const glm::u32vec2& size, const glm::vec4& color) :
        m_Size(size),
        m_Channels(4)  
    {
        size_t byte_count = m_Size.x * m_Size.y * m_Channels;
        std::array<std::byte, 4> rgba = {
            static_cast<std::byte>(static_cast<uint8_t>(color.r * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.g * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.b * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.a * 255.0f))
        };
        m_Data.resize(byte_count);
        for (size_t i = 0; i < byte_count; i += 4) {
            std::memcpy(&m_Data[i], rgba.data(), 4);
        }
    }


    Texture::Texture(const Texture& other) :
        m_Size(other.m_Size),
        m_Channels(other.m_Channels),
        m_Data(other.m_Data)
    {

    }
    
    Texture::Texture(Texture&& other) noexcept :
        m_Size(std::move(other.m_Size)),
        m_Channels(std::move(other.m_Channels)),
        m_Data(std::move(other.m_Data))
    {

    }

    const glm::u32vec2& Texture::size() const {
        return m_Size;
    }

    std::uint32_t Texture::channels() const {
        return m_Channels;
    }

    std::size_t Texture::bytes() const {
        return m_Data.size();
    }

    std::span<const std::byte> Texture::data() const {
        return std::span(m_Data.cbegin(), m_Data.size());
    }

}