#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/instance.hpp"
#include "luna-gfx/error/error.hpp"

namespace luna {
namespace vulkan {

  #if defined(__unix__) || defined(_WIN32)
constexpr const char* END_COLOR = "\x1B[m";
constexpr const char* COLOR_RED = "\u001b[31m";
constexpr const char* COLOR_GREEN = "\u001b[32m";
constexpr const char* COLOR_YELLOW = "\u001b[33m";
constexpr const char* COLOR_GREY = "\u001b[37m";
constexpr const char* UNDERLINE = "\u001b[4m";
#else
constexpr const char* END_COLOR = "";
constexpr const char* COLOR_GREEN = "";
constexpr const char* COLOR_YELLOW = "";
constexpr const char* COLOR_GREY = "";
constexpr const char* COLOR_RED = "";
constexpr const char* COLOR_WHITE = "";
#endif

/** Static Vulkan debug callback for any vulkan instance errors.
 * @param messageSeverity The severity of the debug message.
 * @param messageType The type of message.
 * @param pCallbackData The data containing the callback information.
 * @param pUserData The input user-specified this->
 * @return Whether or not the error was handled.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
  auto severity =
      static_cast<vk::DebugUtilsMessageSeverityFlagsEXT>(messageSeverity);
  auto type = static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType);

  const char* COLOR;
  COLOR = COLOR_YELLOW;

  if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
    return VK_FALSE;
  if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
    return VK_FALSE;
  if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    COLOR = COLOR_YELLOW;
  if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
    COLOR = COLOR_RED;

  std::cout << "\n";
  std::cout << COLOR << "[Luna Instance Debug]" << END_COLOR << "\n";
  std::cout << COLOR << "  Type    : " << vk::to_string(type) << END_COLOR
            << "\n";
  std::cout << COLOR << "  Severity: " << vk::to_string(severity) << END_COLOR
            << "\n";
  std::cout << COLOR << "  Message: " << pCallbackData->pMessage << END_COLOR
            << "\n";

  pUserData = pUserData;
  return VK_FALSE;
}

  Instance::Instance(gfx::Dlloader& loader, vk::AllocationCallbacks* allocation, std::string_view app_name) {
    using StringVec = std::vector<const char*>;

  static bool sdl_init = false;
  unsigned amt = 0;
  std::vector<const char*> ext;
  SDL_Window* dummy;

  if (!sdl_init) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
    sdl_init = true;
  }

  this->app_name = app_name;

#ifdef Luna_Debug
  this->extensions = {"VK_EXT_debug_report"};
#else
  this->extensions = {};
#endif
  this->m_devices = {};

  dummy = SDL_CreateWindow("", 0, 0, 1280, 720,
                           SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN);

  SDL_Vulkan_GetInstanceExtensions(dummy, &amt, nullptr);
  this->extensions.resize(amt);
  ext.resize(amt);
  SDL_Vulkan_GetInstanceExtensions(dummy, &amt, ext.data());

  SDL_DestroyWindow(dummy);

  for (unsigned index = 0; index < ext.size(); index++) {
    this->extensions[index] = ext[index];
  }

  auto info = vk::InstanceCreateInfo();
  vk::ApplicationInfo app_info;
  vk::DebugUtilsMessengerCreateInfoEXT debug_info;
  StringVec extensions;
  StringVec validation;

  // Create initial dispatch so we can query driver for info.
  this->m_dispatch.init(reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      loader.symbol("vkGetInstanceProcAddr")));

  app_info = this->makeAppInfo();
  debug_info = this->makeDebugInfo();
  extensions = this->makeExtensionList();
  validation = this->makeValidationList();

  // Initialize debug instance callback if we have validation layers enabled.
  if (!this->validation.empty()) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    info.setPNext(
        reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_info));
  }

  info.setEnabledLayerCount(validation.size());
  info.setEnabledExtensionCount(extensions.size());
  info.setPpEnabledLayerNames(validation.data());
  info.setPpEnabledExtensionNames(extensions.data());
  info.setPApplicationInfo(&app_info);

  this->m_instance =
      error(vk::createInstance(info, allocation, this->m_dispatch));

  // Reinitialize dispatch with created instance.
  this->m_dispatch.init(this->m_instance,
                        reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                            loader.symbol("vkGetInstanceProcAddr")));

    this->m_devices =
      error(this->m_instance.enumeratePhysicalDevices(this->m_dispatch));

    this->extensions.clear();
    this->validation.clear();
  }


  Instance::~Instance() {
    //if (this->m_instance) {
    //  this->m_instance.destroy(nullptr, this->m_dispatch);
    //  SDL_Quit();
    //}
  }
auto Instance::operator=(Instance&& mv) -> Instance& {
  this->m_devices = mv.m_devices;
  this->m_dispatch = mv.m_dispatch;
  this->m_instance = mv.m_instance;

  mv.m_devices.clear();
  mv.m_instance = nullptr;
  return *this;
}

vk::ApplicationInfo Instance::makeAppInfo() {
  vk::ApplicationInfo info;

  info.setPEngineName(this->app_name.c_str());
  info.setPApplicationName(this->app_name.c_str());
  info.setApiVersion(VK_API_VERSION_1_2);
  info.setEngineVersion(VK_MAKE_VERSION(0, 0, 0));

  return info;
}

vk::DebugUtilsMessengerCreateInfoEXT Instance::makeDebugInfo() {
  const auto severity_bits =
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
  const auto message_type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  vk::DebugUtilsMessengerCreateInfoEXT info;

  info.setMessageSeverity(severity_bits);
  info.setMessageType(message_type);
  info.setPfnUserCallback(debugCallback);

  return info;
}

std::vector<const char*> Instance::makeExtensionList() {
  std::vector<const char*> list;

  std::vector<::vk::ExtensionProperties> available_extentions;

  auto result = error(
      vk::enumerateInstanceExtensionProperties(nullptr, Instance::m_dispatch));
  auto copy = this->extensions;

  available_extentions = result;

  this->extensions.clear();
  for (const auto& ext : available_extentions) {
    for (const auto& requested : copy) {
      if (std::string(&ext.extensionName[0]) == requested) {
        this->extensions.push_back(requested);
        list.push_back(this->extensions.back().data());
      }
    }
  }

  return list;
};

std::vector<const char*> Instance::makeValidationList() {
  std::vector<const char*> list;
  std::vector<::vk::LayerProperties> available_layers;

  auto result =
      error(vk::enumerateInstanceLayerProperties(Instance::m_dispatch));
  auto copy = this->validation;

  // todo:: handle result
  available_layers = result;

  this->validation.clear();
  for (const auto& ext : available_layers) {
    //for (const auto& requested : copy) {
      if (std::string(&ext.layerName[0]).find("KHRONOS") != std::string::npos) {
        this->validation.push_back(ext.layerName);
        list.push_back(this->validation.back().data());
      }
    //}
  }

  return list;
};
}
}