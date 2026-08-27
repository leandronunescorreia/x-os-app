#include "app_framework/framebuffer.h"

#include <cassert>

int main() {
    app_framework::Framebuffer buffer(2, 2);
    assert(buffer.width() == 2);
    assert(buffer.height() == 2);
    assert(buffer.stride() == 8);
    buffer.clear(1, 2, 3, 4);
    assert(buffer.data()[0] == 3);
    assert(buffer.data()[1] == 2);
    assert(buffer.data()[2] == 1);
    assert(buffer.data()[3] == 4);
    buffer.set_pixel(1, 1, 10, 20, 30);
    assert(buffer.data()[12] == 30);
    assert(buffer.data()[13] == 20);
    assert(buffer.data()[14] == 10);
    buffer.set_pixel(9, 9, 0, 0, 0);
    return 0;
}
