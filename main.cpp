#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Assert.hpp"
#include "Log.hpp"
#include "VkDebugHelper.hpp"

class VkRenderer
{
    private:
        VkInstance                  m_Instance  = VK_NULL_HANDLE;
        VkPhysicalDevice            m_GPU       = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties  m_GPUProperties{};
        VkDevice                    m_Device    = VK_NULL_HANDLE;
        VkQueue                     m_Queue     = VK_NULL_HANDLE;
        uint32_t                    m_GraphicsFamilyIndex = 0;

        std::vector<const char*>    m_InstanceLayers;
        std::vector<const char*>    m_InstanceExtensions;

        std::vector<const char*>    m_DeviceLayers;
        std::vector<const char*>    m_DeviceExtensions;

        // TODO: Move it from renderer. And other debug
        VkDebugReportCallbackEXT    m_DebugReport = VK_NULL_HANDLE;
        VkDebugReportCallbackCreateInfoEXT m_DebugReportCallbackCreateInfo{};

        void InitInstance();
        void DeinitInstance();

        void InitDevice();
        void DeinitDevice();

        void SetupLayersAndExtensions();

        void SetupDebug();
        void InitDebug();
        void DeinitDebug();
    public:
        VkRenderer();
        ~VkRenderer();

        [[nodiscard]] VkInstance                    GetVulkanInstance()                 const;
        [[nodiscard]] VkPhysicalDevice              GetVulkanPhysicalDevice()           const;
        [[nodiscard]] VkDevice                      GetVulkanDevice()                   const;
        [[nodiscard]] VkQueue                       GetVulkanQueue()                    const;
        [[nodiscard]] uint32_t                      GetVulkanGraphicsFamilyIndex()      const;
        [[nodiscard]] const VkPhysicalDeviceProperties&   GetVulkanPhysicalDeviceProperties() const;
};



int main()
{
    glfwInit();
    int width = 800;
    int height = 600;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Eternity", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    VkRenderer r;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t surfaceWidth;
    uint32_t surfaceHeight;
    VkResult result = glfwCreateWindowSurface(r.GetVulkanInstance(), window, nullptr, &surface);
    VkDebugHelper::CheckResult(result);

    // init OsSpecificSurface
    {
        VkBool32 WSI_SUPPORTED = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(r.GetVulkanPhysicalDevice(), r.GetVulkanGraphicsFamilyIndex(), surface, &WSI_SUPPORTED);
        ET_CORE_ASSERT(WSI_SUPPORTED, "WSI is not supported");
    }
    // init surface
    VkSurfaceFormatKHR surfaceFormat{};
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    {
        auto gpu = r.GetVulkanPhysicalDevice();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

        if (surfaceCapabilities.currentExtent.width < UINT32_MAX)
        {
            surfaceWidth    = surfaceCapabilities.currentExtent.width;
            surfaceHeight   = surfaceCapabilities.currentExtent.height;
        }

        ET_CORE_ASSERT(formatCount > 0, "Surface formats missing");
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats.data());
        if (formats[0].format == VK_FORMAT_UNDEFINED)
        {
            surfaceFormat.format        = VK_FORMAT_B8G8R8A8_UNORM;
            surfaceFormat.colorSpace    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            surfaceFormat               = formats[0];
        }
    }

    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount = 2;
    // Init swapchain
    {

        if (swapchainImageCount > surfaceCapabilities.maxImageCount ) swapchainImageCount = surfaceCapabilities.maxImageCount;
        if (swapchainImageCount < surfaceCapabilities.minImageCount ) swapchainImageCount = surfaceCapabilities.minImageCount + 1;

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        {
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(r.GetVulkanPhysicalDevice(), surface, &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(r.GetVulkanPhysicalDevice(), surface, &presentModeCount, presentModeList.data());
            for (auto m : presentModeList)
            {
                if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                    presentMode = m;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface                 = surface;
        swapchainCreateInfo.minImageCount           = swapchainImageCount;
        swapchainCreateInfo.imageFormat             = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace         = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent.width       = surfaceWidth;
        swapchainCreateInfo.imageExtent.height      = surfaceHeight;
        swapchainCreateInfo.imageArrayLayers        = 1;
        swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount   = 0;
        swapchainCreateInfo.pQueueFamilyIndices     = VK_NULL_HANDLE;
        swapchainCreateInfo.preTransform            = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode             = presentMode;
        swapchainCreateInfo.clipped                 = VK_TRUE;
        swapchainCreateInfo.oldSwapchain            = VK_NULL_HANDLE;

        VkDebugHelper::CheckResult(vkCreateSwapchainKHR(r.GetVulkanDevice(), &swapchainCreateInfo, nullptr, &swapchain));
        VkDebugHelper::CheckResult(vkGetSwapchainImagesKHR(r.GetVulkanDevice(), swapchain, &swapchainImageCount, nullptr));
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // deinit swapchain
    {
        vkDestroySwapchainKHR(r.GetVulkanDevice(), swapchain, nullptr);
    }

    vkDestroySurfaceKHR(r.GetVulkanInstance(), surface, nullptr);
    glfwTerminate();
    return VK_SUCCESS;
}

VkRenderer::VkRenderer()
{
    SetupDebug();
    SetupLayersAndExtensions();
    InitInstance();
    InitDebug();
    InitDevice();
}

VkRenderer::~VkRenderer()
{
    DeinitDevice();
    DeinitDebug();
    DeinitInstance();
}

void VkRenderer::InitInstance()
{
    // TODO: think about move it from here its not good idea.
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion                  = VK_MAKE_VERSION(1, 0 ,2);
    applicationInfo.engineVersion               = VK_MAKE_VERSION(0, 0, 4);
    applicationInfo.pEngineName                 = "Eternity";

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo         = &applicationInfo;
    // debug options
    instanceCreateInfo.enabledLayerCount        = m_InstanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames      = m_InstanceLayers.data();
    instanceCreateInfo.enabledExtensionCount    = m_InstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames  = m_InstanceExtensions.data();
    instanceCreateInfo.pNext                    = &m_DebugReportCallbackCreateInfo;

    // TODO: Read about other parameters of this structure
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);

    VkDebugHelper::CheckResult(result);
}

void VkRenderer::DeinitInstance()
{
    vkDestroyInstance(m_Instance, nullptr);
}

// needed to create surface
void VkRenderer::SetupLayersAndExtensions()
{
    uint32_t extensionsCount				= 0;
    const char ** instanceExtensionsBuffer		= glfwGetRequiredInstanceExtensions(&extensionsCount);
    for(uint32_t i = 0; i < extensionsCount; i++)
        m_InstanceExtensions.push_back(instanceExtensionsBuffer[i]);

    m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

// Create physical and logical device
// find gpu and create logical device to manipulate this
void VkRenderer::InitDevice()
{
    // TODO: Generally its not good idea to store it in one method. its better to decompose it
    {
        uint32_t gpuCount = 0; // count physical devices
        vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);
        // after that we have amount of gpu's
        std::vector<VkPhysicalDevice> gpuList(gpuCount);
        // call again to save gpu handles
        vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpuList.data());
        // TODO: select the best, not take first gpu from list
        m_GPU = gpuList.at(0);
        vkGetPhysicalDeviceProperties(m_GPU, &m_GPUProperties);
    }
    {
        // describe queue families that we will use
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> familyPropertiesList{familyCount};
        vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, familyPropertiesList.data());

        bool found = false;
        for (uint32_t i = 0; i < familyCount; i++)
        {
            if (familyPropertiesList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                found = true;
                m_GraphicsFamilyIndex = i;
            }
        }
        ET_CORE_ASSERT(found, "Queue family supporting graphics not found");
    }

    float queuePriorities[] { 1.0f };
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_GraphicsFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    // debug options
    deviceCreateInfo.enabledLayerCount        = m_DeviceLayers.size();
    deviceCreateInfo.ppEnabledLayerNames      = m_DeviceLayers.data();
    deviceCreateInfo.enabledExtensionCount    = m_DeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames  = m_DeviceExtensions.data();

    VkResult result = vkCreateDevice(m_GPU, &deviceCreateInfo, nullptr, &m_Device);
    VkDebugHelper::CheckResult(result);

    // This is very minimal and simple setup
    // TODO: Read about all properties of all structures to create it more advanced way
    vkGetDeviceQueue(m_Device, m_GraphicsFamilyIndex, 0, &m_Queue);
}

void VkRenderer::DeinitDevice()
{
    vkDestroyDevice(m_Device, nullptr);
}

#ifdef ET_ENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugReportFlagsEXT       flags,
                    VkDebugReportObjectTypeEXT  objType,
                    uint64_t                    sourceObjs,
                    size_t                      location,
                    int32_t                     msgCode,
                    const char*                 layerPrefix,
                    const char*                 msg,
                    void*                       userData)
{
    std::string log = "[ " + std::string(layerPrefix) + " ] " + msg;
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        ET_CORE_INFO(log);
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        ET_CORE_WARN(log);
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        ET_CORE_PERFORMANCE(log);
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        ET_CORE_ERROR(log);
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        ET_CORE_TRACE(log);
    return VK_FALSE;
}

void VkRenderer::SetupDebug()
{
    m_DebugReportCallbackCreateInfo.sType         = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    m_DebugReportCallbackCreateInfo.pfnCallback   = VulkanDebugCallback;
    m_DebugReportCallbackCreateInfo.flags         = VK_DEBUG_REPORT_INFORMATION_BIT_EXT         |
                                                    VK_DEBUG_REPORT_WARNING_BIT_EXT             |
                                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                    VK_DEBUG_REPORT_ERROR_BIT_EXT               |
                                                    VK_DEBUG_REPORT_DEBUG_BIT_EXT               |
                                                    0;

    m_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    m_InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    m_DeviceLayers.push_back("VK_LAYER_KHRONOS_validation");
}

PFN_vkCreateDebugReportCallbackEXT fVkCreateDebugReportCallbackEXT      = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT fVkDestroyDebugReportCallbackEXT    = VK_NULL_HANDLE;

void VkRenderer::InitDebug()
{
    fVkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
    fVkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
    ET_CORE_ASSERT(fVkCreateDebugReportCallbackEXT != VK_NULL_HANDLE &&
                    fVkDestroyDebugReportCallbackEXT != VK_NULL_HANDLE, "Can't fetch debug function pointers");
    fVkCreateDebugReportCallbackEXT(m_Instance, &m_DebugReportCallbackCreateInfo, nullptr, &m_DebugReport);
}

void VkRenderer::DeinitDebug()
{
    fVkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, nullptr);
}

#else
void VkRenderer::SetupDebug() {}
void VkRenderer::InitDebug() {}
void VkRenderer::DeinitDebug() {}
#endif // ET_ENABLE_VULKAN_DEBUG


VkInstance                    VkRenderer::GetVulkanInstance()                 const { return m_Instance; }
VkPhysicalDevice              VkRenderer::GetVulkanPhysicalDevice()           const { return m_GPU; };
VkDevice                      VkRenderer::GetVulkanDevice()                   const { return m_Device; }
VkQueue                       VkRenderer::GetVulkanQueue()                    const { return m_Queue; }
uint32_t                      VkRenderer::GetVulkanGraphicsFamilyIndex()      const { return m_GraphicsFamilyIndex; }
const VkPhysicalDeviceProperties&   VkRenderer::GetVulkanPhysicalDeviceProperties() const { return m_GPUProperties; }