#include "app_framework/framebuffer.h"

#include <algorithm>
#include <limits>

namespace app_framework {

Framebuffer::Framebuffer(std::uint32_t width, std::uint32_t height) {
    resize(width, height);
}

bool Framebuffer::resize(std::uint32_t width, std::uint32_t height) {
    const std::uint64_t byte_count = static_cast<std::uint64_t>(width) * height * 4;
    if (byte_count > std::numeric_limits<std::size_t>::max()) {
        return false;
    }

    try {
        pixels_.assign(static_cast<std::size_t>(byte_count), 0);
    } catch (...) {
        return false;
    }
    width_ = width;
    height_ = height;
    return true;
}

void Framebuffer::clear(std::uint8_t red, std::uint8_t green,
                        std::uint8_t blue, std::uint8_t alpha) {
    for (std::size_t index = 0; index < pixels_.size(); index += 4) {
        pixels_[index] = blue;
        pixels_[index + 1] = green;
        pixels_[index + 2] = red;
        pixels_[index + 3] = alpha;
    }
}

void Framebuffer::set_pixel(std::uint32_t x, std::uint32_t y,
                            std::uint8_t red, std::uint8_t green,
                            std::uint8_t blue, std::uint8_t alpha) {
    if (x >= width_ || y >= height_) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(y) * stride() + x * 4;
    pixels_[index] = blue;
    pixels_[index + 1] = green;
    pixels_[index + 2] = red;
    pixels_[index + 3] = alpha;
}

} // namespace app_framework
