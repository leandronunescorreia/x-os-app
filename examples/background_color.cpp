#include "app_framework/app.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <algorithm>

class BackgroundColor final : public app_framework::App {
public:
    void on_message(const app_framework::WindowMessage& message) override {
        if (message.type == app_framework::MessageType::close) {
            window()->close();
        }
    }

protected:
    void update() override {
        auto& buffer = window()->framebuffer();

        const std::uint32_t width  = buffer.width();
        const std::uint32_t height = buffer.height();

        // Time since the application started.
        const auto now = std::chrono::steady_clock::now();

        const float time =
            std::chrono::duration<float>(
                now.time_since_epoch()
            ).count();

        // Speed of the horizontal movement.
        constexpr float scroll_speed = 120.0f;

        // How fast the colors change.
        constexpr float color_speed = 1.0f;

        // Horizontal offset. Increasing this moves the pattern left.
        const float offset = time * scroll_speed;

        for (std::uint32_t y = 0; y < height; ++y) {
            for (std::uint32_t x = 0; x < width; ++x) {

                // Normalize the coordinates.
                const float fx =
                    static_cast<float>(x) / static_cast<float>(width);

                const float fy =
                    static_cast<float>(y) / static_cast<float>(height);

                // Create a moving wave.
                const float wave =
                    std::sin(
                        (static_cast<float>(x) + offset) * 0.015f +
                        fy * 4.0f
                    );

                // Time-dependent color phase.
                const float phase = time * color_speed;

                // Generate smoothly changing RGB channels.
                const float r =
                    0.5f +
                    0.5f * std::sin(wave * 2.0f + phase);

                const float g =
                    0.5f +
                    0.5f * std::sin(wave * 2.0f + phase + 2.094f);

                const float b =
                    0.5f +
                    0.5f * std::sin(wave * 2.0f + phase + 4.188f);

                buffer.set_pixel(
                    x,
                    y,
                    static_cast<std::uint8_t>(r * 255.0f),
                    static_cast<std::uint8_t>(g * 255.0f),
                    static_cast<std::uint8_t>(b * 255.0f)
                );
            }
        }
    }
};

int start() {
    BackgroundColor app;
    if (!app.initialize({"App Framework Example", 800, 450})) return 1;
    const int result = app.run();
    app.shutdown();
    return result;    
}


#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    return start();
}
#else
int main() {
    return start();
}
#endif