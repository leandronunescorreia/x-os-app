#pragma once

#include "app_framework/framebuffer.h"

#include <cstdint>
#include <memory>
#include <string>

namespace app_framework {

enum class MessageType {
    close,
    resize,
    key_down,
    key_up,
    mouse_move,
    mouse_button_down,
    mouse_button_up
};

struct WindowMessage {
    MessageType type;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t key = 0;
    std::uint32_t button = 0;
};

class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void on_message(const WindowMessage& message) = 0;
};

struct WindowConfig {
    std::string title = "Application";
    std::uint32_t width = 960;
    std::uint32_t height = 540;
};

class Window {
public:
    virtual ~Window() = default;
    virtual bool create(const WindowConfig& config) = 0;
    virtual void poll_messages() = 0;
    virtual void present() = 0;
    virtual void close() = 0;
    virtual bool is_open() const noexcept = 0;
    virtual Framebuffer& framebuffer() noexcept = 0;
    virtual void set_message_handler(MessageHandler* handler) noexcept = 0;
};

std::unique_ptr<Window> create_window();

} // namespace app_framework
