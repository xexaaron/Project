#include "Core/App.h"
#include "Core/Log.h"
#include "Rendering/UI/Widget.h"
#include "vk/VkRenderer.h"
#include "Platform/Platform.h"

// Static data
namespace aby {
    
    fs::path App::s_Path = sys::get_exec_path();

    fs::path App::bin() {
        return s_Path.parent_path();
    }

    const fs::path& App::exe() {
        return s_Path;
    }

    fs::path App::cache() {
        return bin() / "Cache";
    }
}

namespace aby {

    App::App(const AppInfo& app_info, const WindowInfo& window_info) : 
        m_Window(Window::create(WindowInfo{
            .size  = window_info.size,
            .flags = window_info.flags,
            .title = (app_info.binherit ? app_info.name : window_info.title)
        })),
        m_Ctx(Context::create(this, m_Window.get())),
        m_Renderer(Renderer::create(m_Ctx)),
        m_Info(app_info)
    {
        m_Window->register_event(this, &App::on_event);
        fs::path object_cache = cache() / "Objects";
        if (!std::filesystem::exists(object_cache)) {
            std::filesystem::create_directories(object_cache);
        }
    }

    App::~App() {
        m_Renderer->destroy();
        m_Ctx->destroy();
    }

    void App::run() {
        m_Ctx->load_thread().sync();
        while (m_Ctx->load_thread().tasks() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m_Ctx->load_thread().sync();

        auto object_cache = cache() / "Objects";
        for (auto& obj : m_Objects) {
            auto file = object_cache / std::to_string(obj->uuid()) / ".bin";
            if (std::filesystem::exists(file)) {
                SerializeOpts opts{
                  .file = file,
                  .mode = ESerializeMode::READ
                };
                Serializer serializer(opts);
                bool deserialized = obj->on_deserialize(serializer);
                obj->on_create(this, deserialized);
            }
            else {
                obj->on_create(this, false);
            }
        }

        m_Window->initalize();

        for (auto& object : m_Objects) {
            if (auto p = std::dynamic_pointer_cast<ui::Widget>(object)) {
                p->on_invalidate();
            }
        }

        auto last_time = std::chrono::high_resolution_clock::now();
        float delta_time = 0.0f;
        while (!m_Window->is_open()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            if (!m_Window->is_minimized()) {
                for (auto& obj : m_Objects) {
                    obj->on_tick(this, Time(delta_time));
                }
                m_Window->swap_buffers();
                m_Window->poll_events();
            }
            Logger::flush();
        }

        for (auto& obj : m_Objects) {
            SerializeOpts opts{
                .file = object_cache / std::to_string(obj->uuid()) / ".bin",
                .mode = ESerializeMode::WRITE
            };
            Serializer serializer(opts);
            obj->on_serialize(serializer);
            serializer.save();
            obj->on_destroy(this);
        }
    }
 
    void App::set_name(const std::string& name) {
        m_Info.name = name;
        if (m_Info.binherit) {
            m_Window->set_title(name);
        }
    }

    const std::string& App::name() const {
        return m_Info.name;
    }

    const AppVersion& App::version() const {
        return m_Info.version;
    }

    Window* App::window() {
        return m_Window.get();
    }
    Window* App::window() const {
        return m_Window.get();
    }

    Context& App::ctx() {
        return *m_Ctx;
    }
    const Context& App::ctx() const {
        return *m_Ctx;
    }

    Renderer& App::renderer() {
        return *m_Renderer;
    }

    const Renderer& App::renderer() const {
        return *m_Renderer;
    }

    std::span<Ref<Object>> App::objects() {
        return std::span(m_Objects.begin(), m_Objects.size());
    }

    std::span<const Ref<Object>> App::objects() const {
        return std::span(m_Objects.cbegin(), m_Objects.size());
    }

    void App::add_object(Ref<Object> obj) {
        m_Objects.push_back(obj);
    }

    void App::remove_object(Ref<Object> obj) {
        auto it = std::remove(m_Objects.begin(), m_Objects.end(), obj);
        if (it != m_Objects.end()) {
            (*it)->on_destroy(this);
            m_Objects.erase(it, m_Objects.end());
        }
    }
    void App::on_event(Event& event) {
    #if 0
        ABY_LOG("{}", event.to_string());
    #endif
        for (auto& obj : m_Objects) {
            obj->on_event(this, event);
        }
    }

    const AppInfo& App::info() const {
        return m_Info;
    }
 


}