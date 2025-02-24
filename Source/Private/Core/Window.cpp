#include "Core/Window.h"
#include "Core/Log.h"
#include <glfw/glfw3.h>

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <glfw/glfw3native.h>
    #include <dwmapi.h>
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <glfw/glfw3native.h>
#elif defined(__APPLE__)
    #error "Unsupported Platform"
#endif

namespace aby {

    Window::Window(const WindowInfo& info) :
        m_Data{
            .title    = info.title,
            .width    = info.size.x,
            .height   = info.size.y,
            .flags    = info.flags,
            .callback = [](Event&){}
        },
        m_Window(nullptr)
    {
        ABY_ASSERT(glfwInit(), "Failed to initialize GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        glfwSetErrorCallback([](int code, const char* description) {
            ABY_ERR("[GLFW] ({}): {}", code, description);
        });

        m_Window = glfwCreateWindow(info.size.x, info.size.y, info.title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            ABY_ASSERT(Window::m_Window, "Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, this);
        setup_callbacks();

    #ifdef _WIN32
        BOOL use_dark_mode = TRUE;
        DwmSetWindowAttribute(static_cast<HWND>(native()), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));
    #endif
    }

    Unique<Window> Window::create(const WindowInfo& info) {
        return create_unique<Window>(info);
    }

    Window::~Window() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    bool Window::is_open() const {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::initalize() {
        if ((m_Data.flags & EWindowFlags::VSYNC) != EWindowFlags::NONE) {
            set_vsync(true);
        }
        bool maximize = (m_Data.flags & EWindowFlags::MAXIMIZED) != EWindowFlags::NONE;
        bool minimize = (m_Data.flags & EWindowFlags::MINIMIZED) != EWindowFlags::NONE;
        ABY_ASSERT(!(maximize && minimize), "Invalid usage of EWindowFlags");
        if (maximize) {
            set_maximized(true);
        }
        else if (minimize) {
            set_minimized(true);
        }
        int width, height;
        glfwGetWindowSize(m_Window, &width, &height);
        WindowResizeEvent wr_event(width, height, m_Data.width, m_Data.height);
        m_Data.width = width;
        m_Data.height = height;
        m_Data.callback(wr_event);
    }

    void Window::poll_events() const {
        glfwPollEvents();
    }

    void Window::swap_buffers() const {
        glfwSwapBuffers(m_Window);
    }
    
    GLFWwindow* Window::glfw() const {
        return m_Window;
    }
    void* Window::native() const {
    #ifdef _WIN32
            return static_cast<void*>(glfwGetWin32Window(m_Window));
    #elif defined(__linux__)
            return static_cast<void*>(glfwGetX11Window(m_Window));
    #endif
    }
    std::uint32_t Window::width() const {
        return m_Data.width;
    }

    std::uint32_t Window::height() const {
        return m_Data.height;
    }

    glm::u32vec2 Window::size() const {
        return { m_Data.width, m_Data.height };
    }

    double Window::scale() const {
        float x, y;
        glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x, &y);
        return (double)x;
    }

    int Window::refresh_rate() const {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return mode->refreshRate;
    }

    void Window::set_title(const std::string& title) {
        m_Data.title = title;
        glfwSetWindowTitle(m_Window, m_Data.title.c_str());
    }

    void Window::set_size(std::uint32_t w, std::uint32_t h) {
        glfwSetWindowSize(m_Window, w, h);
    }

    void Window::set_position(std::uint32_t x, std::uint32_t y) {
        glfwSetWindowPos(m_Window, x, y);
    }

    void Window::set_minimized(bool minimzed) {
        if (minimzed) {
            glfwIconifyWindow(m_Window);
        }
        else {
            glfwRestoreWindow(m_Window);
        }
    }

    void Window::set_maximized(bool maximized) {
        if (maximized) {
            glfwMaximizeWindow(m_Window);
        }
        else {
            glfwRestoreWindow(m_Window);
        }
    }

    void Window::set_vsync(bool vsync) {
        if (vsync) {
            m_Data.flags |= EWindowFlags::VSYNC;
        }
        else {
            m_Data.flags &= ~EWindowFlags::VSYNC;
        }
        glfwSwapInterval(static_cast<int>(vsync));
    }

    bool Window::is_vsync() const {
        return (m_Data.flags & EWindowFlags::VSYNC) != EWindowFlags::NONE;
    }
    
    bool Window::is_minimized() const {
        return (m_Data.flags & EWindowFlags::MINIMIZED) != EWindowFlags::NONE;
    }
 
    bool Window::is_maximized() const {
        return (m_Data.flags & EWindowFlags::MAXIMIZED) != EWindowFlags::NONE;
    }


    bool Window::is_key_pressed(Button::EKey button) const {
        auto state = glfwGetKey(m_Window, button);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    
    bool Window::is_mouse_pressed(Button::EMouse button) const {
        auto state = glfwGetMouseButton(m_Window, button);
        return state == GLFW_PRESS;
    }
    glm::fvec2 Window::desktop_resolution() const {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return { static_cast<float>(mode->width), static_cast<float>(mode->height) };
    }

    void Window::setup_callbacks() {
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* win, int w, int h) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            WindowResizeEvent event(w, h, data.width, data.height);
            data.width  = w;
            data.height = h;
            data.callback(event);
        });
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* win) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            WindowCloseEvent wc_event;
            data.callback(wc_event);
        });
        glfwSetKeyCallback(m_Window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            switch (action) {
                case GLFW_PRESS:
                {
                    KeyPressedEvent kp_event(static_cast<Button::EKey>(key), 0);
                    data.callback(kp_event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent kr_event(static_cast<Button::EKey>(key));
                    data.callback(kr_event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent kr_event(static_cast<Button::EKey>(key), 1);
                    data.callback(kr_event);
                    break;
                }

            }
        });
        glfwSetCharCallback(m_Window, [](GLFWwindow* win, unsigned int keycode) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            KeyTypedEvent ev(static_cast<Button::EKey>(keycode));
            data.callback(ev);
        });
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* win, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            double xpos, ypos;
            glfwGetCursorPos(win, &xpos, &ypos);
            glm::u32vec2 pos(
                static_cast<std::uint32_t>(xpos),
                static_cast<std::uint32_t>(ypos)
            );
            switch (action) {
                case GLFW_PRESS:
                {
                    MousePressedEvent mp_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mp_event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent mr_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mr_event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    MousePressedEvent mr_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mr_event);
                    break;
                }
            }
        });
        glfwSetScrollCallback(m_Window, [](GLFWwindow* win, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            MouseScrolledEvent ms_event((float)xOffset, (float)yOffset);
            data.callback(ms_event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* win, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            MouseMovedEvent ms_event((float)xPos, (float)yPos);
            data.callback(ms_event);
        });
        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* win, int iconified) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            if (iconified == GLFW_TRUE) {
                data.flags |= EWindowFlags::MINIMIZED;
                data.flags &= ~EWindowFlags::MAXIMIZED;
            }
            else {
                data.flags &= ~EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }

            // Force size synchronization
            int width, height;
            glfwGetWindowSize(win, &width, &height);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(win, &fbWidth, &fbHeight);  // Ensure rendering size is updated

            WindowResizeEvent wr_event(width, height, data.width, data.height);
            data.width = width;
            data.height = height;
            data.callback(wr_event);
            });

        glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* win, int maximized) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(win);
            if (maximized == GLFW_TRUE) {
                data.flags |= EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }
            else {
                data.flags &= ~EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }

            // Force size synchronization
            int width, height;
            glfwGetWindowSize(win, &width, &height);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(win, &fbWidth, &fbHeight);  // Ensure rendering size is updated

            WindowResizeEvent wr_event(width, height, data.width, data.height);
            data.width = width;
            data.height = height;
            data.callback(wr_event);
            });
        m_Data.callback = [this](Event& event) -> void {
            for (auto it = m_Callbacks.rbegin(); it != m_Callbacks.rend(); ++it) {
                (*it)(event);
            }
        };
    }
 
    void Window::register_event(const std::function<void(Event&)>& event) {
        m_Callbacks.push_back(event);
    }

    glm::fvec2 Window::mouse_pos() const {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
        return { static_cast<float>(x), static_cast<float>(y) };
    }

    glm::fvec2 Window::dpi() const {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        ABY_ASSERT(monitor, "Failed to get primary monitor");

        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);

        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        ABY_ASSERT(mode, "Failed to get video mode");
        
        return {
            (mode->width  / (widthMM / 25.4f))  * xscale,
            (mode->height / (heightMM / 25.4f)) * yscale
        };

    }

    void Window::set_cursor(ECursor cursor) {
        GLFWcursor* gcursor = nullptr;
        switch (cursor) {
            case ECursor::ARROW:
                gcursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                break;
            case ECursor::IBEAM:
                gcursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                break;
            case ECursor::CROSSHAIR:
                gcursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
                break;
            case ECursor::HAND:
                gcursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                break;
            case ECursor::HRESIZE:
                gcursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
                break;
            case ECursor::VRESIZE:
                gcursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
                break;
            default:
                gcursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                break;
        }
        if (gcursor) {
            glfwSetCursor(m_Window, gcursor);
        }
    }


}

