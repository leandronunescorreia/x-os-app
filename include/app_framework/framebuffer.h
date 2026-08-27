#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace app_framework {

class Framebuffer {
public:
    Framebuffer() = default;
    Framebuffer(std::uint32_t width, std::uint32_t height);

    bool resize(std::uint32_t width, std::uint32_t height);
    void clear(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255);
    void set_pixel(std::uint32_t x, std::uint32_t y,
                   std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                   std::uint8_t alpha = 255);

    std::uint32_t width() const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::size_t stride() const noexcept { return static_cast<std::size_t>(width_) * 4; }
    std::uint8_t* data() noexcept { return pixels_.data(); }
    const std::uint8_t* data() const noexcept { return pixels_.data(); }
    std::size_t size() const noexcept { return pixels_.size(); }

private:
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::vector<std::uint8_t> pixels_;
};

} // namespace app_framework
