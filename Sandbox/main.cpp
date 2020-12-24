#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <vector>

#include "Base.hpp"
#include "VulkanHelper.hpp"

namespace Eternity
{
    class VulkanRenderer
    {
        private:
            struct QueueFamilyIndices
            {
                std::optional<uint32_t> graphicsFamily;
                bool IsComplete()
                {
                    return graphicsFamily.has_value();
                }
            };

            VkInstance          m_Instance          = VK_NULL_HANDLE;

            VkPhysicalDevice    m_PhysicalDevice    = VK_NULL_HANDLE;
            VkDevice            m_LogicalDevice     = VK_NULL_HANDLE;

            void CreateInstance();
            void DestroyInstance();

            QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device);
            bool IsDeviceSuitable(const VkPhysicalDevice& device);
            void PickPhysicalDevice();

            void CreateLogicalDevice();
            void DestroyLogicalDevice();
        public:
            VulkanRenderer();
            ~VulkanRenderer();
    };

    VulkanRenderer::VulkanRenderer()
    {
        CreateInstance();
        VulkanHelper::CreateDebugMessenger(m_Instance);
        PickPhysicalDevice();
        CreateLogicalDevice();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        DestroyLogicalDevice();
        VulkanHelper::DestroyDebugMessenger(m_Instance);
        DestroyInstance();
    }




    void VulkanRenderer::CreateInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName    = "Hello Triangle";
        appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName         = "No Engine";
        appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion          = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo         = &appInfo;

        auto extensions  = VulkanHelper::GetRequiredExtensions();
        instanceCreateInfo.enabledExtensionCount    = extensions.size();
        instanceCreateInfo.ppEnabledExtensionNames  = extensions.data();

        auto layers = VulkanHelper::GetInstanceLayers();
        instanceCreateInfo.enabledLayerCount        = layers.size();
        instanceCreateInfo.ppEnabledLayerNames      = layers.data();
        instanceCreateInfo.pNext                    = VulkanHelper::GetDebugCreateInfo();
        ET_CORE_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) == VK_SUCCESS, "Instance creation");
    }

    void VulkanRenderer::DestroyInstance()
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    VulkanRenderer::QueueFamilyIndices VulkanRenderer::FindQueueFamilies(const VkPhysicalDevice& device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        // get queue family count
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // get queue family properties
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // now find queue family which we need
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            if (indices.IsComplete())
                break;

            i++;
        }

        return indices;
    }

    bool VulkanRenderer::IsDeviceSuitable(const VkPhysicalDevice& device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);
        return indices.IsComplete();
    }

    void VulkanRenderer::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        // get gpus count last parameter nullptr
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        ET_CORE_ASSERT(deviceCount != 0, "No gpu");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // now enumerate again and save all gpus to vector
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
        // Find suitable gpu from list of all founded
        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                break;
            }
        }
        ET_CORE_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "No suitable gpu");
    }

    void VulkanRenderer::CreateLogicalDevice()
    {

    }

    void VulkanRenderer::DestroyLogicalDevice()
    {

    }
}

int main()
{
    Eternity::VulkanRenderer renderer;

    return EXIT_SUCCESS;
}

