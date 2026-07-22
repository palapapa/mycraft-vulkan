#pragma once
#include "logger.hpp"
#include <GLFW/glfw3.h>
#include <boost/leaf/error.hpp>
#include <boost/leaf/result.hpp>
#include <cstdint>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace mycraft_vulkan {
/// @brief An RAII wrapper for glfw to automatically initialize and terminate
/// it. Since glfw uses global states, there should be only one instance of this
/// class.
class GlfwContext {
  public:
    GlfwContext(const GlfwContext &) = delete;
    auto operator=(const GlfwContext &) -> GlfwContext & = delete;

    GlfwContext(GlfwContext &&other) noexcept : owns_context(std::exchange(other.owns_context, false)) {
    }

    auto operator=(GlfwContext &&other) -> GlfwContext & = delete;

    ~GlfwContext() {
        if (owns_context) {
            glfwTerminate();
        }
    }

    /// @brief Creates @ref GlfwContext.
    ///
    /// @return The value is not present if glfw failed to initialize.
    static auto create() -> std::optional<GlfwContext> {
        if (glfwInit() != GLFW_TRUE) {
            return {};
        }
        return GlfwContext(true);
    }

  private:
    /// @brief Used to prevent glfw being terminated immediately after @ref
    /// GlfwContext is moved.
    bool owns_context = false;
    explicit GlfwContext(bool owns_context) : owns_context(owns_context) {
    }
};

enum class GlfwWindowError : uint8_t { SurfaceCreation };

/// @brief An RAII wrapper for @ref GLFWWindow to automatically create and
/// destroy it.
class GlfwWindow {
  public:
    GlfwWindow(const GlfwWindow &) = delete;
    auto operator=(const GlfwWindow &) -> GlfwWindow & = delete;

    GlfwWindow(GlfwWindow &&other) noexcept : window(std::exchange(other.window, nullptr)) {
    }

    auto operator=(GlfwWindow &&other) noexcept -> GlfwWindow & {
        if (this == &other) {
            return *this;
        }
        glfwDestroyWindow(window);
        window = other.window;
        other.window = nullptr;
        return *this;
    }

    ~GlfwWindow() {
        glfwDestroyWindow(window);
    }

    /// @brief Creates @ref GlfwWindow. The arguments are passed straight to
    /// @ref glfwCreateWindow. If you need to set window hints, set them using
    /// @ref glfwWindowHint before calling this.
    ///
    /// @return The value is not present if the window failed to create.
    static auto create(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share) -> std::optional<GlfwWindow> {
        auto window = GlfwWindow(glfwCreateWindow(width, height, title, monitor, share));
        if (!window.window) {
            LOG_ERROR("Could not create a GlfwWindow.");
            return {};
        }
        return window;
    }

    auto window_should_close() -> int {
        return glfwWindowShouldClose(window);
    }

    auto create_surface(const VkInstance &instance) const -> boost::leaf::result<VkSurfaceKHR> {
        auto *surface = VkSurfaceKHR();
        const auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Could not create a VkSurfaceKHR from a GlfwWindow. Error: {}", result);
            return BOOST_LEAF_NEW_ERROR(GlfwWindowError::SurfaceCreation, result);
        }
        return surface;
    }

  private:
    GLFWwindow *window = nullptr;
    explicit GlfwWindow(GLFWwindow *window) : window(window) {
    }
};

inline auto glfw_error_callback(int code, const char *description) -> void {
    LOG_ERROR("GLFW error. Error code: {}. Error message: {}", code, description);
}
} // namespace mycraft_vulkan
