#pragma once
#include "constants.hpp"
#include "glfw-helpers.hpp"
#include "leaf-healpers.hpp"
#include "logger.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <algorithm>
#include <array>
#include <boost/leaf/error.hpp>
#include <boost/leaf/result.hpp>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <quill/HelperMacros.h>
#include <quill/std/Vector.h> // IWYU pragma: keep
#include <ranges>
#include <utility>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>

namespace mycraft_vulkan {
enum class AppCreationError : uint8_t {
    WindowCreation,
    InstanceCreation,
    DebugMessengerCreation,
    SurfaceCreation,
    PhysicalDeviceCreation,
    DeviceCreation,
    SwapchainCreation,
    SwapchainImagesCreation,
    GraphicsQueueCreation
};

class App {
  public:
    auto run() -> void {
        main_loop();
    }

    static auto create() -> boost::leaf::result<App> {
        auto window = create_window();
        if (!window) {
            LOG_ERROR("Could not create glfw window.");
            return BOOST_LEAF_NEW_ERROR(AppCreationError::WindowCreation);
        }
        auto context = vk::raii::Context();
        BOOST_LEAF_AUTO(vkb_instance, create_instance());
        BOOST_LEAF_AUTO(vk_surface, add_error(window->create_surface(vkb_instance), AppCreationError::SurfaceCreation));
        BOOST_LEAF_AUTO(vkb_physical_device, create_physical_device(vkb_instance, vk_surface));
        BOOST_LEAF_AUTO(vkb_device, create_device(vkb_physical_device));
        BOOST_LEAF_AUTO(vkb_swapchain, create_swapchain(vkb_device));
        auto instance = vk::raii::Instance(context, vkb_instance.instance);
        auto debug_messenger = vk::raii::DebugUtilsMessengerEXT(instance, vkb_instance.debug_messenger);
        auto surface = vk::raii::SurfaceKHR(instance, vk_surface);
        auto physical_device = vk::raii::PhysicalDevice(instance, std::move(vkb_physical_device));
        auto device = vk::raii::Device(physical_device, vkb_device.device);
        auto vk_graphics_queue_result = vkb_device.get_queue(vkb::QueueType::graphics);
        if (!vk_graphics_queue_result) {
            LOG_ERROR("Could not create the graphics queue. Error: {}", vk_graphics_queue_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::GraphicsQueueCreation, vk_graphics_queue_result.full_error());
        }
        auto graphics_queue = vk::raii::Queue(device, *vk_graphics_queue_result);
        auto swapchain = vk::raii::SwapchainKHR(device, vkb_swapchain.swapchain);
        auto vk_swapchain_images_result = vkb_swapchain.get_images();
        if (!vk_swapchain_images_result) {
            LOG_ERROR("Could not get the swapchain images. Error: {}", vk_swapchain_images_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::SwapchainImagesCreation, vk_swapchain_images_result.full_error());
        }
        auto swapchain_images = *vk_swapchain_images_result | std::views::transform([](VkImage image) { return vk::Image{image}; }) |
                                std::ranges::to<std::vector<vk::Image>>();
        return App(std::move(*window), std::move(context), std::move(instance), std::move(debug_messenger), std::move(surface),
                   std::move(physical_device), std::move(device), std::move(graphics_queue), std::move(swapchain), std::move(swapchain_images));
    }

  private:
    GlfwWindow window;
    vk::raii::Context context;
    vk::raii::Instance instance;
    vk::raii::DebugUtilsMessengerEXT debug_messenger;
    vk::raii::SurfaceKHR surface;
    vk::raii::PhysicalDevice physical_device;
    vk::raii::Device device;
    vk::raii::Queue graphics_queue;
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchain_images;
    static constexpr auto WINDOW_HEIGHT = 600;
    static constexpr auto WINDOW_WIDTH = 800;
    static constexpr auto REQUIRED_DEVICE_EXTENSIONS = std::array<const char *, 0>{};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
    static constexpr auto REQUIRED_DEVICE_FEATURES = VkPhysicalDeviceFeatures{};
    static constexpr auto REQUIRED_DEVICE_FEATURES_11 = VkPhysicalDeviceVulkan11Features{.shaderDrawParameters = VK_TRUE};
    static constexpr auto REQUIRED_DEVICE_FEATURES_12 = VkPhysicalDeviceVulkan12Features{};
    static constexpr auto REQUIRED_DEVICE_FEATURES_13 = VkPhysicalDeviceVulkan13Features{.dynamicRendering = VK_TRUE};
    static constexpr auto REQUIRED_DEVICE_FEATURES_14 = VkPhysicalDeviceVulkan14Features{};
#pragma clang diagnostic pop

    App(GlfwWindow &&window, vk::raii::Context &&context, vk::raii::Instance &&instance, vk::raii::DebugUtilsMessengerEXT &&debug_messenger,
        vk::raii::SurfaceKHR &&surface, vk::raii::PhysicalDevice &&physical_device, vk::raii::Device &&device, vk::raii::Queue &&graphics_queue,
        vk::raii::SwapchainKHR &&swapchain, std::vector<vk::Image> &&swapchain_images)
        : window(std::move(window)), context(std::move(context)), instance(std::move(instance)), debug_messenger(std::move(debug_messenger)),
          surface(std::move(surface)), physical_device(std::move(physical_device)), device(std::move(device)),
          graphics_queue(std::move(graphics_queue)), swapchain(std::move(swapchain)), swapchain_images(std::move(swapchain_images)) {
    }

    static auto create_window() -> std::optional<GlfwWindow> {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        return GlfwWindow::create(WINDOW_WIDTH, WINDOW_HEIGHT, "mycraft-vulkan", nullptr, nullptr);
    }

    auto main_loop() -> void {
        while (window.window_should_close() == 0) {
            glfwPollEvents();
        }
    }

    static auto create_instance() -> boost::leaf::result<vkb::Instance> {
        auto instance_builder = vkb::InstanceBuilder();
        instance_builder.set_app_name("mycraft-vulkan")
            .set_app_version(0, 0)
            .set_engine_name("Custom")
            .set_engine_version(0, 0)
            .require_api_version(1, 4);
        uint32_t glfw_required_extension_count = 0;
        const auto *const *glfw_required_extensions = glfwGetRequiredInstanceExtensions(&glfw_required_extension_count);
        if (!glfw_required_extensions) {
            LOG_ERROR("No set of Vulkan extensions allowing window surface creation was found.");
            return BOOST_LEAF_NEW_ERROR(AppCreationError::InstanceCreation);
        }
        // If glfwGetRequiredInstanceExtensions returns non-null, all the
        // extensions contained should be available.
        instance_builder.enable_extensions(glfw_required_extension_count, glfw_required_extensions);
#ifndef NDEBUG
        const auto message_severity = log_level_env_var_to_vk_debug_utils_message_serverity_flags(
            std::getenv(LOG_LEVEL_ENV_VAR_NAME), vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        const auto message_type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        instance_builder.request_validation_layers()
            .set_debug_callback(debug_callback)
            .set_debug_messenger_severity(static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(message_severity))
            .set_debug_messenger_type(static_cast<VkDebugUtilsMessageTypeFlagsEXT>(message_type));
#endif
        auto instance_result = instance_builder.build();
        if (!instance_result) {
            LOG_ERROR("Could not create the Vulkan Instance. Error: {}.", instance_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::InstanceCreation, instance_result.full_error());
        }
        return *instance_result;
    }

    static VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void * /*unused*/) -> vk::Bool32 {
        const auto message_type = vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type));
        switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity)) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            LOG_ERROR("Validation: {}: {}", message_type, callback_data->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            LOG_WARNING("Validation: {}: {}", message_type, callback_data->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            LOG_INFO("Validation: {}: {}", message_type, callback_data->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            LOG_DEBUG("Validation: {}: {}", message_type, callback_data->pMessage);
            break;
        }
        return vk::False;
    }

    static auto create_physical_device(const vkb::Instance &instance, const VkSurfaceKHR &surface) -> boost::leaf::result<vkb::PhysicalDevice> {
        auto physical_device_selector = vkb::PhysicalDeviceSelector(instance, surface)
                                            .set_minimum_version(1, 4)
                                            .add_required_extensions(REQUIRED_DEVICE_EXTENSIONS.size(), REQUIRED_DEVICE_EXTENSIONS.data())
                                            .set_required_features(REQUIRED_DEVICE_FEATURES)
                                            .set_required_features_11(REQUIRED_DEVICE_FEATURES_11)
                                            .set_required_features_12(REQUIRED_DEVICE_FEATURES_12)
                                            .set_required_features_13(REQUIRED_DEVICE_FEATURES_13)
                                            .set_required_features_14(REQUIRED_DEVICE_FEATURES_14);
        const auto candidate_physical_device_names_result = physical_device_selector.select_device_names();
        if (!candidate_physical_device_names_result) {
            LOG_WARNING("Could not get candidate physical device names. Error: {}", candidate_physical_device_names_result.full_error());
        } else {
            LOG_INFO("Candidate physical devices: {}", *candidate_physical_device_names_result);
        }
        auto candidate_physical_devices_result = physical_device_selector.select_devices();
        if (!candidate_physical_devices_result) {
            LOG_ERROR("Could not enumerate candidate physical device. Error: {}", candidate_physical_devices_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::PhysicalDeviceCreation, candidate_physical_devices_result.full_error());
        }
        auto candidate_physical_devices = std::move(*candidate_physical_devices_result);
        auto suitable_physical_device = std::ranges::find_if(candidate_physical_devices, [](const auto &candidate_physical_device) {
            const auto families = candidate_physical_device.get_queue_families();
            return std::ranges::any_of(families,
                                       [](const auto &family) { return family.queueCount > 0 && (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0; });
        });
        if (suitable_physical_device == candidate_physical_devices.end()) {
            LOG_ERROR("No physical device supports graphics operations.");
            return BOOST_LEAF_NEW_ERROR(AppCreationError::PhysicalDeviceCreation);
        }
        LOG_INFO("Selected physical device: {}", suitable_physical_device->name);
        return std::move(*suitable_physical_device);
    }

    static auto create_device(const vkb::PhysicalDevice &physical_device) -> boost::leaf::result<vkb::Device> {
        const auto device_builder = vkb::DeviceBuilder(physical_device);
        auto device_result = device_builder.build();
        if (!device_result) {
            LOG_ERROR("Could not create device. Error: {}", device_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::PhysicalDeviceCreation, device_result.full_error());
        }
        return std::move(*device_result);
    }

    static auto create_swapchain(const vkb::Device &device) -> boost::leaf::result<vkb::Swapchain> {
        const auto swapchain_builder = vkb::SwapchainBuilder(device);
        auto swapchain_result = swapchain_builder.build();
        if (!swapchain_result) {
            LOG_ERROR("Could not create the swapchain. Error: {}", swapchain_result.full_error());
            return BOOST_LEAF_NEW_ERROR(AppCreationError::SwapchainCreation, swapchain_result.full_error());
        }
        return *swapchain_result;
    }
};
} // namespace mycraft_vulkan

QUILL_LOGGABLE_DEFERRED_FORMAT(mycraft_vulkan::AppCreationError)
