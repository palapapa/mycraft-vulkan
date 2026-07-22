#include "app.hpp"
#include "glfw-helpers.hpp"
#include "leaf-healpers.hpp"
#include "logger.hpp"
#include <GLFW/glfw3.h>
#include <boost/leaf/error.hpp>
#include <boost/leaf/result.hpp>
#include <cstdlib>

auto main() -> int {
    using namespace mycraft_vulkan; // NOLINT(google-build-using-namespace)
    LOG_INFO("App starting");
    LOG_INFO("Using GLFW version {}.", glfwGetVersionString());
    glfwSetErrorCallback(glfw_error_callback);
    auto glfw_context = GlfwContext::create();
    if (!glfw_context) {
        LOG_ERROR("GLFW failed to initialize.");
        return EXIT_FAILURE;
    }
    if (glfwVulkanSupported() == GLFW_FALSE) {
        LOG_ERROR("Vulkan is not available.");
        return EXIT_FAILURE;
    }
    return try_handle_all_relaxed(
        []() -> boost::leaf::result<int> {
            BOOST_LEAF_AUTO(app, App::create());
            app.run();
            LOG_INFO("App exiting.");
            return EXIT_SUCCESS;
        },
        [](AppCreationError error) -> int {
            LOG_ERROR("App creation failed. Error: {}", error);
            return EXIT_FAILURE;
        });
}
