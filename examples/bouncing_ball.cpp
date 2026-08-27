#include "app_framework/app.h"

#include <cstdint>
#include <cmath>
#include <chrono>

class BouncingBallApp final : public app_framework::App {
public:
    void on_message(const app_framework::WindowMessage& message) override {
        if (message.type == app_framework::MessageType::close) {
            window()->close();
        }
    }

protected:
    void backGroundAnimation(const std::uint32_t width, const std::uint32_t height, app_framework::Framebuffer& fb) {
        // Animation time
        const auto now = std::chrono::steady_clock::now();
        const float time = std::chrono::duration<float>(now.time_since_epoch()).count();

        // Background animation parameters
        constexpr float scroll_speed = 120.0f;
        constexpr float wave_scale = 0.015f;
        constexpr float color_speed = 1.0f;

        const float offset = time * scroll_speed;
        const float phase = time * color_speed;

        // Animated background
        for (std::uint32_t y = 0; y < height; ++y) {
            const float fy = static_cast<float>(y) / static_cast<float>(height);

            for (std::uint32_t x = 0; x < width; ++x) {
                const float wave = std::sin((static_cast<float>(x) + offset) * wave_scale + fy * 4.0f);

                const float r = 0.5f + 0.5f * std::sin(wave * 2.0f + phase);

                const float g = 0.5f + 0.5f * std::sin( wave * 2.0f + phase + 2.094f );

                const float b = 0.5f + 0.5f * std::sin( wave * 2.0f + phase + 4.188f );

                fb.set_pixel(
                    x,
                    y,
                    static_cast<std::uint8_t>(r * 255.0f),
                    static_cast<std::uint8_t>(g * 255.0f),
                    static_cast<std::uint8_t>(b * 255.0f)
                );
            }
        }
    }

    void update() override {
        auto& fb = window()->framebuffer();

        const std::uint32_t width = fb.width();
        const std::uint32_t height = fb.height();

        backGroundAnimation(width, height, fb);

        // Ball properties
        constexpr std::int32_t ball_radius = 60; // signed, so loop/comparisons below stay signed

        // Update position using per-axis velocity
        ball_x += ball_x_speed;
        ball_y += ball_y_speed;

        // Bounce off walls (clamp position + flip velocity, once per axis)
        if (ball_x + ball_radius > width) {
            ball_x = width - ball_radius;
            ball_x_speed = -ball_x_speed;
        } else if (ball_x - ball_radius < 0) {
            ball_x = ball_radius;
            ball_x_speed = -ball_x_speed;
        }

        if (ball_y + ball_radius > height) {
            ball_y = height - ball_radius;
            ball_y_speed = -ball_y_speed;
        } else if (ball_y - ball_radius < 0) {
            ball_y = ball_radius;
            ball_y_speed = -ball_y_speed;
        }

        // Draw ball (circle using midpoint algorithm approximation)
        const std::int32_t cx = static_cast<std::int32_t>(ball_x);
        const std::int32_t cy = static_cast<std::int32_t>(ball_y);

        for (std::int32_t dy = -ball_radius; dy <= ball_radius; ++dy) {
            for (std::int32_t dx = -ball_radius; dx <= ball_radius; ++dx) {
                if (dx * dx + dy * dy <= ball_radius * ball_radius) {
                    const std::int32_t px_signed = cx + dx;
                    const std::int32_t py_signed = cy + dy;
                    if (px_signed >= 0 && py_signed >= 0) {
                        const std::uint32_t px = static_cast<std::uint32_t>(px_signed);
                        const std::uint32_t py = static_cast<std::uint32_t>(py_signed);
                        if (px < width && py < height) {
                            // Red ball with slight gradient
                            const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                            const float brightness = 1.0f - (dist / ball_radius) * 0.3f;
                            fb.set_pixel(px, py,
                                static_cast<std::uint8_t>(255 * brightness),
                                static_cast<std::uint8_t>(50 * brightness),
                                static_cast<std::uint8_t>(50 * brightness));
                        }
                    }
                }
            }
        }
    }

private:
    float ball_x = 100.0f;
    float ball_y = 100.0f;
    float ball_x_speed = 3.0f;
    float ball_y_speed = 3.0f;

};

int start() {
    BouncingBallApp app;
    if (!app.initialize({"Bouncing Ball - Double Buffering Demo", 800, 450})) {
        return 1;
    }
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