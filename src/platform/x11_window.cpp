#include "app_framework/window.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cstring>
#include <memory>

namespace app_framework {
namespace {

class X11Window final : public Window {
public:
    ~X11Window() override { close(); }

    bool create(const WindowConfig& config) override {
        display_ = XOpenDisplay(nullptr);
        if (!display_) return false;
        screen_ = DefaultScreen(display_);
        window_ = XCreateSimpleWindow(display_, RootWindow(display_, screen_), 0, 0,
                                      config.width, config.height, 1,
                                      BlackPixel(display_, screen_), WhitePixel(display_, screen_));
        if (!window_) return false;
        title_ = config.title;
        XStoreName(display_, window_, title_.c_str());
        delete_atom_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display_, window_, &delete_atom_, 1);
        XSelectInput(display_, window_, ExposureMask | StructureNotifyMask |
                     KeyPressMask | KeyReleaseMask | PointerMotionMask |
                     ButtonPressMask | ButtonReleaseMask);
        gc_ = XCreateGC(display_, window_, 0, nullptr);
        if (!framebuffer_.resize(config.width, config.height)) return false;
        XMapWindow(display_, window_);
        open_ = true;
        return true;
    }

    void poll_messages() override {
        while (display_ && XPending(display_)) {
            XEvent event;
            XNextEvent(display_, &event);
            WindowMessage message{};
            bool notify = true;
            switch (event.type) {
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) != delete_atom_) {
                    notify = false;
                    break;
                }
                message.type = MessageType::close;
                break;
            case ConfigureNotify:
                message.type = MessageType::resize;
                message.width = static_cast<std::uint32_t>(event.xconfigure.width);
                message.height = static_cast<std::uint32_t>(event.xconfigure.height);
                framebuffer_.resize(message.width, message.height);
                break;
            case MotionNotify:
                message.type = MessageType::mouse_move;
                message.x = event.xmotion.x;
                message.y = event.xmotion.y;
                break;
            case ButtonPress:
                message.type = MessageType::mouse_button_down;
                message.button = event.xbutton.button;
                message.x = event.xbutton.x;
                message.y = event.xbutton.y;
                break;
            case ButtonRelease:
                message.type = MessageType::mouse_button_up;
                message.button = event.xbutton.button;
                message.x = event.xbutton.x;
                message.y = event.xbutton.y;
                break;
            case KeyPress:
                message.type = MessageType::key_down;
                message.key = event.xkey.keycode;
                break;
            case KeyRelease:
                message.type = MessageType::key_up;
                message.key = event.xkey.keycode;
                break;
            default:
                notify = false;
                break;
            }
            if (notify && handler_) handler_->on_message(message);
        }
    }

    void present() override {
        if (!open_ || !display_) return;
        XImage* image = XCreateImage(display_, DefaultVisual(display_, screen_),
                                     DefaultDepth(display_, screen_), ZPixmap, 0,
                                     reinterpret_cast<char*>(framebuffer_.data()),
                                     framebuffer_.width(), framebuffer_.height(), 32, framebuffer_.stride());
        if (!image) return;
        XPutImage(display_, window_, gc_, image, 0, 0, 0, 0,
                  framebuffer_.width(), framebuffer_.height());
        image->data = nullptr;
        XDestroyImage(image);
        XFlush(display_);
    }

    void close() override {
        if (display_) {
            if (gc_) XFreeGC(display_, gc_);
            if (window_) XDestroyWindow(display_, window_);
            XCloseDisplay(display_);
        }
        display_ = nullptr;
        gc_ = 0;
        window_ = 0;
        open_ = false;
    }

    bool is_open() const noexcept override { return open_; }
    Framebuffer& framebuffer() noexcept override { return framebuffer_; }
    void set_message_handler(MessageHandler* handler) noexcept override { handler_ = handler; }

private:
    Display* display_ = nullptr;
    ::Window window_ = 0;
    GC gc_ = 0;
    Atom delete_atom_ = 0;
    int screen_ = 0;
    std::string title_;
    bool open_ = false;
    Framebuffer framebuffer_;
    MessageHandler* handler_ = nullptr;
};

} // namespace

std::unique_ptr<Window> create_window() {
    return std::make_unique<X11Window>();
}

} // namespace app_framework
