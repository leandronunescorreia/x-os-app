#include "app_framework/window.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <memory>

namespace app_framework {
namespace {

class Win32Window final : public Window {
public:
    ~Win32Window() override { close(); }

    bool create(const WindowConfig& config) override {
        instance_ = GetModuleHandleW(nullptr);
        class_name_ = L"AppFrameworkWindow";
        WNDCLASSEXW window_class{sizeof(WNDCLASSEXW)};
        window_class.hInstance = instance_;
        window_class.lpfnWndProc = &Win32Window::window_proc;
        window_class.lpszClassName = class_name_;
        window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        if (!RegisterClassExW(&window_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;

        const int width = static_cast<int>(config.width);
        const int height = static_cast<int>(config.height);
        hwnd_ = CreateWindowExW(0, class_name_, widen(config.title).c_str(), WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                                nullptr, nullptr, instance_, this);
        if (!hwnd_ || !framebuffer_.resize(config.width, config.height)) return false;
        ShowWindow(hwnd_, SW_SHOW);
        open_ = true;
        return true;
    }

    void poll_messages() override {
        MSG message;
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                WindowMessage close_message{MessageType::close};
                if (handler_) handler_->on_message(close_message);
                open_ = false;
            } else {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
        }
    }

    void present() override {
        if (!hwnd_ || !open_) return;
        HDC dc = GetDC(hwnd_);
        if (!dc) return;
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = static_cast<LONG>(framebuffer_.width());
        info.bmiHeader.biHeight = -static_cast<LONG>(framebuffer_.height());
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        RECT client;
        GetClientRect(hwnd_, &client);
        StretchDIBits(dc, 0, 0, client.right, client.bottom, 0, 0,
                      framebuffer_.width(), framebuffer_.height(), framebuffer_.data(),
                      &info, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hwnd_, dc);
    }

    void close() override {
        if (hwnd_) DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        open_ = false;
    }

    bool is_open() const noexcept override { return open_; }
    Framebuffer& framebuffer() noexcept override { return framebuffer_; }
    void set_message_handler(MessageHandler* handler) noexcept override { handler_ = handler; }

private:
    static std::wstring widen(const std::string& text) {
        return std::wstring(text.begin(), text.end());
    }

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        auto* self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            self = static_cast<Win32Window*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
        }
        if (!self) return DefWindowProcW(hwnd, message, wparam, lparam);

        WindowMessage event{};
        bool notify = true;
        switch (message) {
        case WM_CLOSE:
            event.type = MessageType::close;
            self->open_ = false;
            DestroyWindow(hwnd);
            break;
        case WM_SIZE:
            event.type = MessageType::resize;
            event.width = LOWORD(lparam);
            event.height = HIWORD(lparam);
            self->framebuffer_.resize(event.width, event.height);
            break;
        case WM_MOUSEMOVE:
            event.type = MessageType::mouse_move; event.x = GET_X_LPARAM(lparam); event.y = GET_Y_LPARAM(lparam); break;
        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
            event.type = MessageType::mouse_button_down; event.button = message; break;
        case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
            event.type = MessageType::mouse_button_up; event.button = message; break;
        case WM_KEYDOWN: event.type = MessageType::key_down; event.key = static_cast<std::uint32_t>(wparam); break;
        case WM_KEYUP: event.type = MessageType::key_up; event.key = static_cast<std::uint32_t>(wparam); break;
        case WM_DESTROY:
            self->open_ = false; PostQuitMessage(0); notify = false; break;
        default: notify = false; break;
        }
        if (notify && self->handler_) self->handler_->on_message(event);
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    LPCWSTR class_name_ = nullptr;
    bool open_ = false;
    Framebuffer framebuffer_;
    MessageHandler* handler_ = nullptr;
};

} // namespace

std::unique_ptr<Window> create_window() {
    return std::make_unique<Win32Window>();
}

} // namespace app_framework
