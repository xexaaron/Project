#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include "Rendering/Texture.h"
#include <unordered_map>
#include <fstream>

// Font
namespace aby {

    class Context;

    class Font {
    public:
        struct Glyph {
            std::uint32_t advance;
            std::uint32_t offset;
            glm::vec2     bearing;
            glm::vec2     size;  
            glm::vec2     texcoords[4];
        };
        using Glyphs = std::unordered_map<char32_t, Glyph>;
    public:
        static Resource create(Context* ctx, const fs::path& path, std::uint32_t pt = 14);
        ~Font();

        Resource      texture() const;
        std::string   name() const;
        std::uint32_t size() const;
        const Glyphs& glyphs() const;
        glm::vec2     measure(const std::string& text) const;
        float         text_height() const;
    protected:
        Font(Context* ctx, const fs::path& path, const glm::vec2& dpi, std::uint32_t pt = 14);
    private:
        std::uint32_t m_SizePt;
        float         m_TextHeight;
        Resource      m_Texture;
        std::string   m_Name;
        Glyphs        m_Glyphs;
    };


}