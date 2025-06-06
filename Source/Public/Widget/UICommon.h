#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <glm/glm.hpp>

namespace aby {
    class App;
}

namespace aby::ui {
    
    enum class EAnchor {
        NONE          = 0,
        TOP_LEFT      = 1, 
        TOP_CENTER    = 2,    
        TOP_RIGHT     = 3,    
        CENTER_LEFT   = 4,
        CENTER        = 5,
        CENTER_RIGHT  = 6,
        BOTTOM_LEFT   = 7,
        BOTTOM_CENTER = 8,
        BOTTOM_RIGHT  = 9,
    };
  
    enum class EDirection {
        HORIZONTAL = 0, // x
        VERTICAL   = 1, // y
    };

    enum class ELayout {
        AUTO = 0, // If horizontal then left to right, else vertical, then top to bottom.
        LEFT_TO_RIGHT,
        TOP_TO_BOTTOM,
        RIGHT_TO_LEFT,
        BOTTOM_TO_TOP
    };

    enum class EButtonState {
        DEFAULT  = 0,
        HOVERED  = 1,
        PRESSED  = 2,
        RELEASED = DEFAULT,
    };

    enum class ETextAlignment {
        CENTER = 0,
        LEFT,
        RIGHT,
    };

    enum class EResize {
        NONE = 0,
        N    = BIT(0),
        E    = BIT(1),
        S    = BIT(2),
        W    = BIT(3),
    };

    DECLARE_ENUM_OPS(EResize);
}

namespace aby::ui {
    struct Anchor {
        EAnchor   position = EAnchor::NONE;
        glm::vec2 offset   = { 0.f, 0.f };
    };
    struct Transform {
        Anchor    anchor   = {};
        glm::vec2 position = { 0.f, 0.f };
        glm::vec2 size     = { 0.f, 0.f };
    };
    
    struct Background {
        glm::vec4 color = { 1.f, 1.f, 1.f, 1.f }; 
        Resource  texture = {};
    };
    struct Border {
        glm::vec4 color = { 0.f, 0.f, 0.f, 1.f };
        float     width = 0.f;
    };

    struct ImageStyle {
        Border    border;
        glm::vec4 color;
        Resource  texture;

        static ImageStyle dark_mode();   // Default dark mode style.
        static ImageStyle light_mode();  // Default light mode style.
    };

    struct ButtonStyle {
        Background hovered  = {}; 
        Background pressed  = {};
        Background released = {}; // released and default are the same.
        Border     border   = {};
        
        static ButtonStyle dark_mode();   // Default dark mode style.
        static ButtonStyle light_mode();  // Default light mode style.
    };

    struct TextInfo {
        std::string    text      = "";
        glm::vec4      color     = { 1, 1, 1, 1 };
        float          scale     = 1.f;
        ETextAlignment alignment = ETextAlignment::CENTER;
    };

    struct TextCursor {
        std::size_t pos     = std::string::npos;
        char        cursor  = '_';
    };

    struct InputTextOptions {
        bool submit_clears_focus = false; // Does pressing enter/submitting make the input text box lose focus.
        bool submit_clears_text  = true;  // Does pressing enter/submitting clear the text (not including prefix).
        bool cursor              = true;  // Is a cursor shown when the textbox is focused.
        bool unused              = false; // padding for now, might get turned into an option later.
    };

    struct ResizeResult {
        float distance;
        EResize direction; // N, E -> Grow. S, W -> Shrink
    };

    class ResizeOperation {
    public:
        ResizeOperation(EResize resizability);
        ~ResizeOperation() = default;

        bool begin(const glm::vec2& mouse_pos);
        void update(App* app, const Transform& transform, const glm::vec2& mouse_pos, float pad = 0.f);
        ResizeResult resize(Transform& transform, const glm::vec2& mouse_pos);
        void reset();

        bool can_resize() const;
        bool is_resizing() const;
        bool should_resize() const;

        void set_resizability(EResize resizability);
    private:
        bool      is      = false;          // Determines if a resize operation is occurring.
        EResize   ability = EResize::NONE;  // Determines the edges allowed to be resized.
        EResize   state   = EResize::NONE;  // Determines which edge is currently being resized
        ECursor   cursor  = ECursor::ARROW; // The cursor to set on a resize operation.
        glm::vec2 start   = { 0, 0 };       // The start position of the mouse.
        glm::vec2 end     = { 0, 0 };       // The end position of the mouse.
    };
}

namespace std {
    string to_string(aby::ui::EAnchor);
}