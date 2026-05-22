#include "logger.hpp"

auto main() -> int {
    setup_quill();
    LOG_INFO("App starting");
    LOG_INFO("App exiting.");
}
