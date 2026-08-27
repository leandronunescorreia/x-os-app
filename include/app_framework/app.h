#pragma once

#include "app_framework/window.h"

#include <memory>

namespace app_framework {

class App : public MessageHandler {
public:
    virtual ~App() = default;
    bool initialize(const WindowConfig& config = {});
    int run();
    void shutdown();
    Window* window() noexcept { return window_.get(); }
    const Window* window() const noexcept { return window_.get(); }

protected:
    virtual void update() {}

private:
    void on_message(const WindowMessage& message) override;
    std::unique_ptr<Window> window_;
};

} // namespace app_framework
