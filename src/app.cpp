#include "app_framework/app.h"

namespace app_framework {

bool App::initialize(const WindowConfig& config) {
    window_ = create_window();
    return window_ && window_->create(config) && (window_->set_message_handler(this), true);
}

int App::run() {
    if (!window_ || !window_->is_open()) {
        return 1;
    }
    while (window_->is_open()) {
        window_->poll_messages();
        if (window_->is_open()) {
            update();
            window_->present();
        }
    }
    return 0;
}

void App::shutdown() {
    if (window_) {
        window_->close();
        window_.reset();
    }
}

void App::on_message(const WindowMessage& message) {
    if (message.type == MessageType::close && window_) {
        window_->close();
    }
}

} // namespace app_framework
