// window_cocoa.cpp
//
// Cocoa window backend implemented WITHOUT any Objective-C syntax.
// No @interface / @implementation / [obj msg] anywhere below — everything
// goes through the Objective-C runtime's plain C API, so this file can be
// compiled as ordinary C++ (clang++ -x c++, or renamed .cpp in any build
// system that doesn't special-case .mm files). You still need to link
// against libobjc and the Cocoa/CoreGraphics frameworks at link time:
//
//   clang++ -std=c++17 -x objective-c++ ... <-- NOT needed
//   clang++ -std=c++17 window_cocoa.cpp -framework Cocoa -framework CoreGraphics -lobjc -o app
//
// (clang is happy to compile this as plain C++; only the *frameworks*
// remain Objective-C under the hood.)

#include "app_framework/window.h"

#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <cstring>
#include <memory>

namespace app_framework {
namespace {

// ---------------------------------------------------------------------
// Minimal typed wrapper around objc_msgSend.
//
// Casting objc_msgSend to a function pointer with the *real* signature of
// the call (return type + argument types) is what makes this whole approach
// work: the compiler then generates the correct calling convention for the
// call site, including struct-by-value returns like CGRect/NSRect, which is
// normally the job of the Objective-C frontend.
// ---------------------------------------------------------------------
template <typename Ret, typename... Args>
Ret objc_call(id target, SEL sel, Args... args) {
    using Fn = Ret (*)(id, SEL, Args...);
    return reinterpret_cast<Fn>(objc_msgSend)(target, sel, args...);
}

// Convenience for allocating + init'ing a plain NSObject-derived instance.
id objc_alloc(const char* class_name) {
    id cls = reinterpret_cast<id>(objc_getClass(class_name));
    return objc_call<id>(cls, sel_registerName("alloc"));
}

SEL S(const char* name) { return sel_registerName(name); }
Class C(const char* name) { return objc_getClass(name); }

id make_nsstring(const char* utf8) {
    id str = objc_alloc("NSString");
    return objc_call<id>(str, S("initWithUTF8String:"), utf8);
}

// AppKit style-mask / backing-store constants (stable ABI values, normally
// pulled in via the AppKit headers -- we just hardcode them here since we
// aren't including any Objective-C headers).
constexpr unsigned long NSWindowStyleMaskTitled = 1 << 0;
constexpr unsigned long NSWindowStyleMaskClosable = 1 << 1;
constexpr unsigned long NSWindowStyleMaskResizable = 1 << 3;
constexpr unsigned long NSBackingStoreBuffered = 2;
constexpr unsigned long NSEventMaskAny = ~0ul;
constexpr long NSApplicationActivationPolicyRegular = 0;
constexpr unsigned long NSEventTypeApplicationDefined = 15;

// ---------------------------------------------------------------------
// Runtime-defined "AppFrameworkView : NSView" replacement.
//
// Instead of @interface/@implementation we build the class object by hand
// and attach a C function as the drawRect: implementation. State that used
// to live in an @property now lives in an ivar, set/read via
// object_setInstanceVariable / object_getInstanceVariable so we don't need
// to know its byte offset.
// ---------------------------------------------------------------------
void view_drawRect(id self_, SEL /*_cmd*/, CGRect /*dirtyRect*/) {
    void* fb_ptr = nullptr;
    object_getInstanceVariable(self_, "framebuffer", &fb_ptr);
    Framebuffer* framebuffer = static_cast<Framebuffer*>(fb_ptr);
    if (!framebuffer || framebuffer->width() == 0) return;

    id graphics_context = objc_call<id>(C("NSGraphicsContext"), S("currentContext"));
    CGContextRef context = objc_call<CGContextRef>(graphics_context, S("CGContext"));

    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(
        nullptr, framebuffer->data(), framebuffer->size(), nullptr);
    CGImageRef image = CGImageCreate(
        framebuffer->width(), framebuffer->height(), 8, 32, framebuffer->stride(), color_space,
        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst, provider, nullptr, false,
        kCGRenderingIntentDefault);

    CGRect bounds = objc_call<CGRect>(self_, S("bounds"));
    CGContextDrawImage(context, CGRectMake(0, 0, bounds.size.width, bounds.size.height), image);

    CGImageRelease(image);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
}

Class build_view_class() {
    Class superclass = C("NSView");
    Class cls = objc_allocateClassPair(superclass, "AppFrameworkView", 0);
    class_addIvar(cls, "framebuffer", sizeof(void*), static_cast<uint8_t>(alignof(void*)), "^v");
    // Encoding string only needs to be roughly right; the runtime doesn't
    // hard-validate it the way the compiler would.
    class_addMethod(cls, S("drawRect:"), reinterpret_cast<IMP>(&view_drawRect), "v@:{CGRect={CGPoint=dd}{CGSize=dd}}");
    objc_registerClassPair(cls);
    return cls;
}

// ---------------------------------------------------------------------
// Runtime-defined "AppFrameworkDelegate : NSObject <NSWindowDelegate>"
// replacement.
// ---------------------------------------------------------------------
void delegate_windowWillClose(id self_, SEL /*_cmd*/, id /*notification*/) {
    void* handler_ptr = nullptr;
    object_getInstanceVariable(self_, "handler", &handler_ptr);
    auto* handler = static_cast<MessageHandler*>(handler_ptr);
    if (handler) {
        WindowMessage message{MessageType::close};
        handler->on_message(message);
    }
}

Class build_delegate_class() {
    Class superclass = C("NSObject");
    Class cls = objc_allocateClassPair(superclass, "AppFrameworkDelegate", 0);
    class_addIvar(cls, "handler", sizeof(void*), static_cast<uint8_t>(alignof(void*)), "^v");
    class_addMethod(cls, S("windowWillClose:"), reinterpret_cast<IMP>(&delegate_windowWillClose), "v@:@");
    objc_registerClassPair(cls);
    return cls;
}

// Registers both runtime classes exactly once per process.
void ensure_classes_registered() {
    static bool done = false;
    if (done) return;
    build_view_class();
    build_delegate_class();
    done = true;
}

void set_ivar_ptr(id obj, const char* name, void* value) {
    object_setInstanceVariable(obj, name, value);
}

// ---------------------------------------------------------------------
// CocoaWindow — same public shape as the original, internals now go
// through objc_call<>() instead of Objective-C message syntax.
// ---------------------------------------------------------------------
class CocoaWindow final : public Window {
    
public:
    ~CocoaWindow() override { close(); }

    bool create(const WindowConfig& config) override {
        ensure_classes_registered();

        // [NSApplication sharedApplication]
        objc_call<id>(C("NSApplication"), S("sharedApplication"));
        id app = objc_call<id>(C("NSApplication"), S("sharedApplication"));
        objc_call<void>(app, S("setActivationPolicy:"), NSApplicationActivationPolicyRegular);

        CGRect frame = CGRectMake(0, 0, config.width, config.height);

        // window_ = [[NSWindow alloc] initWithContentRect:...]
        id window_alloc = objc_alloc("NSWindow");
        unsigned long style_mask =
            NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
        window_ = objc_call<id>(window_alloc,
                                 S("initWithContentRect:styleMask:backing:defer:"),
                                 frame, style_mask, NSBackingStoreBuffered, /*defer=*/(BOOL)NO);

        if (!window_ || !framebuffer_.resize(config.width, config.height)) return false;

        // view_ = [[AppFrameworkView alloc] initWithFrame:frame]
        id view_alloc = objc_call<id>(reinterpret_cast<id>(objc_getClass("AppFrameworkView")), S("alloc"));
        view_ = objc_call<id>(view_alloc, S("initWithFrame:"), frame);
        set_ivar_ptr(view_, "framebuffer", &framebuffer_);

        // delegate_ = [[AppFrameworkDelegate alloc] init]
        id delegate_alloc = objc_call<id>(reinterpret_cast<id>(objc_getClass("AppFrameworkDelegate")), S("alloc"));
        delegate_ = objc_call<id>(delegate_alloc, S("init"));
        set_ivar_ptr(delegate_, "handler", handler_);

        objc_call<void>(window_, S("setDelegate:"), delegate_);
        objc_call<void>(window_, S("setContentView:"), view_);

        id title = make_nsstring(config.title.c_str());
        objc_call<void>(window_, S("setTitle:"), title);

        objc_call<void>(window_, S("makeKeyAndOrderFront:"), (id) nullptr);
        objc_call<void>(app, S("activateIgnoringOtherApps:"), (BOOL) YES);

        open_ = true;
        return true;
    }

    void poll_messages() override {
        id app = objc_call<id>(C("NSApplication"), S("sharedApplication"));
        id distant_past = objc_call<id>(C("NSDate"), S("distantPast"));
        id default_mode = make_nsstring("kCFRunLoopDefaultMode");

        for (;;) {
            id event = objc_call<id>(
                app, S("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                NSEventMaskAny, distant_past, default_mode, (BOOL) YES);
            if (!event) break;

            unsigned long type = objc_call<unsigned long>(event, S("type"));
            if (type == NSEventTypeApplicationDefined) continue;

            objc_call<void>(app, S("sendEvent:"), event);

            BOOL visible = objc_call<BOOL>(window_, S("isVisible"));
            if (!visible) open_ = false;
        }
    }

    void present() override { objc_call<void>(view_, S("setNeedsDisplay:"), (BOOL) YES); }

    void close() override {
        if (!open_ && !window_) return;
        open_ = false;
        if (window_) objc_call<void>(window_, S("close"));
        view_ = nullptr;
        window_ = nullptr;
    }

    bool is_open() const noexcept override { return open_; }
    Framebuffer& framebuffer() noexcept override { return framebuffer_; }
    void set_message_handler(MessageHandler* handler) noexcept override {
        handler_ = handler;
        if (delegate_) set_ivar_ptr(delegate_, "handler", handler);
    }

private:
    id window_ = nullptr;
    id view_ = nullptr;
    id delegate_ = nullptr;
    Framebuffer framebuffer_;
    MessageHandler* handler_ = nullptr;
    bool open_ = false;
};

}  // namespace

std::unique_ptr<Window> create_window() { return std::make_unique<CocoaWindow>(); }

}  // namespace app_framework