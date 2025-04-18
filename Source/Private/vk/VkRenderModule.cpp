#include "vk/VkRenderModule.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <unordered_set>

namespace aby::vk {

    RenderPrimitive::RenderPrimitive(Ref<vk::Context> ctx, const ShaderDescriptor& vertex_descriptor, const PrimitiveDescriptor& primitive_descriptor) :
        m_VertexClass(vertex_descriptor, primitive_descriptor.MaxVertices, 0),
        m_VertexAccumulator(m_VertexClass),
        m_VertexBuffer(m_VertexClass, ctx->devices()),
        m_IndexBuffer(primitive_descriptor.MaxIndices * sizeof(std::uint32_t), ctx->devices()),
        m_Descriptor(primitive_descriptor),
        m_IndexCount(0)
    {
       
    }

    void RenderPrimitive::destroy() {
        m_VertexBuffer.destroy();
        if (m_Descriptor.IndicesPer != m_Descriptor.VerticesPer) {
            m_IndexBuffer.destroy();
        }
    }

    const vk::PrimitiveDescriptor& RenderPrimitive::descriptor() const {
        return m_Descriptor;
    }

    std::size_t RenderPrimitive::index_count() const {
        return m_IndexCount;
    }

    std::size_t RenderPrimitive::vertex_count() const {
        return m_VertexAccumulator.count();
    }

    void RenderPrimitive::set_index_data(const std::uint32_t* indices, DeviceManager& manager) {
        ABY_ASSERT(m_Descriptor.IndicesPer != m_Descriptor.VerticesPer, "No index buffer will be used to draw this primitive");
        m_IndexBuffer.set_data(indices, sizeof(std::uint32_t) * m_Descriptor.MaxIndices, manager);
    }

    bool RenderPrimitive::empty() const {
        return this->vertex_count() == 0;
    }

    bool RenderPrimitive::should_flush() const {
        bool flush = m_VertexAccumulator.count() + m_Descriptor.VerticesPer >= m_VertexAccumulator.capacity();
        return flush;
    }

    bool RenderPrimitive::should_flush(std::size_t requested_primitives) const {
        bool flush = m_VertexAccumulator.count() + (m_Descriptor.VerticesPer * requested_primitives) >= m_VertexAccumulator.capacity();
        return flush;
    }


    void RenderPrimitive::bind(VkCommandBuffer cmd, DeviceManager& manager) {
        m_VertexBuffer.set_data(m_VertexAccumulator.data(), m_VertexAccumulator.bytes(), manager);
        m_VertexBuffer.bind(cmd);
        if (m_Descriptor.IndicesPer != m_Descriptor.VerticesPer) {
            m_IndexBuffer.bind(cmd);
        }
    }
    
    void RenderPrimitive::draw(VkCommandBuffer cmd) {
        if (m_Descriptor.IndicesPer == m_Descriptor.VerticesPer) {
            draw_nonindexed(cmd);
        }
        else {
            draw_indexed(cmd);
        }
    }


    void RenderPrimitive::draw_indexed(VkCommandBuffer cmd) {
        vkCmdDrawIndexed(cmd, static_cast<std::uint32_t>(m_IndexCount), 1u, 0u, 0u, 0u);
    }

    void RenderPrimitive::draw_nonindexed(VkCommandBuffer cmd) {
        vkCmdDraw(cmd, static_cast<std::uint32_t>(this->vertex_count()), 1u, 0u, 0u);
    }


    void RenderPrimitive::reset() {
        m_VertexAccumulator.reset();
        m_IndexCount = 0;
    }


}

namespace aby::vk {

    static constexpr glm::mat4 UNIT_MATRIX = glm::mat4(1);

    static constexpr glm::vec2 COORDS[4] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
    static constexpr glm::vec4 VERTEX_POSITIONS[4] = {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f }
    };

    RenderModule::RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders) :
        m_Ctx(ctx.get()),
        m_Module(ShaderModule::create(ctx.get(), shaders[0], shaders[1])),
        m_Pipeline(ctx->window(), ctx->devices(), m_Module, swapchain),
        m_Primitives{
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000,
                .MaxIndices = 30000, 
                .IndicesPer = 3,
                .VerticesPer = 3
            }), // Triangles
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000 * 2,
                .MaxIndices = 60000 * 2,
                .IndicesPer = 6,
                .VerticesPer = 4
            }) // Quads
        }
    {
        auto& quad_prim = this->quads();
        auto& prim_desc = quad_prim.descriptor();
    #ifdef NDEBUG
        uint32_t* indices = new uint32_t[prim_desc.MaxIndices];
        for (uint32_t i = 0, offset = 0; i < prim_desc.MaxIndices; i += prim_desc.IndicesPer, offset += prim_desc.VerticesPer) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
        }
        quad_prim.set_index_data(indices, ctx->devices());
        delete[] indices;
    #else
        std::vector<uint32_t> indices(prim_desc.MaxIndices);
        for (uint32_t i = 0, offset = 0; i < prim_desc.MaxIndices; i += prim_desc.IndicesPer, offset += prim_desc.VerticesPer) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
        }
        quad_prim.set_index_data(indices.data(), ctx->devices());
    #endif
    }

    void RenderModule::destroy() {
        m_Pipeline.destroy();
        for (auto& prim : m_Primitives) {
            prim.destroy();
        }
    }

    void RenderModule::reset() {
        for (auto& prim : m_Primitives) {
            prim.reset();
        }
    }
    void RenderModule::flush(VkCommandBuffer cmd, DeviceManager& manager, ERenderPrimitive primitive) {
        if (primitive == ERenderPrimitive::ALL) {
            for (auto& prim : m_Primitives) {
                if (!prim.empty()) {
                    prim.bind(cmd, manager);
                    prim.draw(cmd);
                }
            }
        }
        else {
            auto& prim = m_Primitives[static_cast<std::size_t>(primitive)];
            if (!prim.empty()) {
                prim.bind(cmd, manager);
                prim.draw(cmd);
            }
        }
    }

    void RenderModule::set_uniforms(const void* data, std::size_t bytes, std::uint32_t binding) {
        m_Module->set_uniforms(data, bytes, binding);
    }

    void RenderModule::draw_triangle(const Triangle& triangle) {
        auto& acc = this->tris();
        acc = triangle.v1;
        ++acc;
        acc = triangle.v2;
        ++acc;
        acc = triangle.v3;
        ++acc;
    }
    void RenderModule::draw_quad(const Quad& quad) {
        auto& acc = this->quads();

        glm::vec3 size = { quad.size.x, quad.size.y, 1.f };
        glm::mat4 transform = glm::translate(UNIT_MATRIX, quad.v.pos) * glm::scale(UNIT_MATRIX, size);

        for (std::size_t i = 0; i < std::size(VERTEX_POSITIONS); i++) {
            glm::vec3 pos(transform * VERTEX_POSITIONS[i]);
            glm::vec3 texinfo(COORDS[i], quad.v.texinfo.z);
            Vertex v(pos, quad.v.col, texinfo);
            acc = v;
            ++acc;
        }
    }
    void RenderModule::draw_text(const Text& text) {
        Ref<Font> font_obj = m_Ctx->fonts().at({ EResource::FONT, text.font });
        float texture = static_cast<float>(font_obj->texture().handle());
        Ref<Texture> tex_obj = m_Ctx->textures().at(font_obj->texture());
        auto& acc = this->quads();
        const auto& glyphs = font_obj->glyphs();

        glm::vec2 text_size = font_obj->measure(text.text) * text.scale;
        glm::vec3 current_position = { text.pos.x, text.pos.y, 0.f }; 
        glm::vec4 color = text.color;

        static const std::unordered_set<char32_t> escape_chars = {
            0x27, 0x22, 0x3f, 0x5c, 0x07, 0x08,
            0x0c, 0x0a, 0x0d, 0x09, 0x0b,
        };

        for (char32_t c : text.text) {
            // Skip escape characters
            if (escape_chars.contains(c)) {
                if (c == U'\t') {
                    current_position.x += (glyphs.at(U' ').advance * text.scale * 4);
                }
                continue;
            }

            // Ensure glyph exists
            auto it = glyphs.find(c);
            if (it == glyphs.end()) continue;
            const auto& g = it->second;

            // Compute size and position
            glm::vec3 size = { g.size.x * text.scale, g.size.y * text.scale, 0.f };
            glm::vec3 pos = {
                (current_position.x + g.bearing.x * text.scale) + (size.x / 2),
                (current_position.y + (text_size.y - g.bearing.y) * text.scale) + (size.y / 2),
                0.f
            };

            // Apply transformations
            glm::mat4 transform = glm::translate(UNIT_MATRIX, pos) * glm::scale(UNIT_MATRIX, size);

            // Vertex processing
            for (std::size_t i = 0; i < 4; i++) {
                glm::vec3 position(transform * VERTEX_POSITIONS[i]);
                glm::vec3 texinfo(g.texcoords[i], texture);
                acc = Vertex(position, color, texinfo);
                ++acc;
            }

            current_position.x += g.advance * text.scale;
        }
    }


    RenderPrimitive& RenderModule::quads() {
        std::size_t idx = static_cast<std::size_t>(ERenderPrimitive::QUAD);
        return m_Primitives[idx];
    }
    RenderPrimitive& RenderModule::tris() {
        std::size_t idx = static_cast<std::size_t>(ERenderPrimitive::TRIANGLE);
        return m_Primitives[idx];
    }

    Ref<ShaderModule> RenderModule::module() const {
        return m_Module;
    }

    vk::Pipeline& RenderModule::pipeline() {
        return m_Pipeline;
    }
  



}