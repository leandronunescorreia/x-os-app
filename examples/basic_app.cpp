#include "app_framework/app.h"

#include <cstdint>

class BasicApp final : public app_framework::App {
public:
    void on_message(const app_framework::WindowMessage& message) override {
        if (message.type == app_framework::MessageType::close) {
            window()->close();
        }
    }

protected:
    void update() override {
        auto& buffer = window()->framebuffer();
        const std::uint32_t width = buffer.width();
        const std::uint32_t height = buffer.height();
        for (std::uint32_t y = 0; y < height; ++y) {
            for (std::uint32_t x = 0; x < width; ++x) {
                buffer.set_pixel(x, y,
                    static_cast<std::uint8_t>((x * 255) / (width ? width : 1)),
                    static_cast<std::uint8_t>((y * 255) / (height ? height : 1)), 80);
            }
        }
    }
};

int main() {
    BasicApp app;
    if (!app.initialize({"App Framework Example", 800, 450})) return 1;
    const int result = app.run();
    app.shutdown();
    return result;
}
