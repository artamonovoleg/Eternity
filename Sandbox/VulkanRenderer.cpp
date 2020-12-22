//
// Created by artamonovoleg on 21.12.2020.
//

#include <GLFW/glfw3.h>
#include "VulkanRenderer.hpp"
#include "VulkanDebug.hpp"
#include "Base.hpp"

namespace Eternity
{
    VulkanRenderer::VulkanRenderer()
    {
        VulkanDebug::SetupDebug(m_InstanceLayers, m_InstanceExtensions, m_DeviceLayers);
        SetupLayersAndExtensions();
        InitInstance();
        VulkanDebug::InitDebug(m_Instance);
        InitDevice();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        DeinitDevice();
        VulkanDebug::DeinitDebug(m_Instance);
        DeinitInstance();
    }

    void VulkanRenderer::InitInstance()
    {
        // TODO: think about move it from here its not good idea.
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);
        applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 4);
        applicationInfo.pEngineName = "Eternity";

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        // debug options
        instanceCreateInfo.enabledLayerCount = m_InstanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = m_InstanceLayers.data();
        instanceCreateInfo.enabledExtensionCount = m_InstanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.data();
        instanceCreateInfo.pNext = VulkanDebug::GetDebugReportCallbackInfo();

        // TODO: Read about other parameters of this structure
        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);

        VulkanDebug::CheckResult(result);
    }

    void VulkanRenderer::DeinitInstance()
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    // needed to create surface
    void VulkanRenderer::SetupLayersAndExtensions()
    {
        uint32_t extensionsCount = 0;
        const char **instanceExtensionsBuffer = glfwGetRequiredInstanceExtensions(&extensionsCount);
        for (uint32_t i = 0; i < extensionsCount; i++)
            m_InstanceExtensions.push_back(instanceExtensionsBuffer[i]);

        m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // Create physical and logical device
    // find gpu and create logical device to manipulate this
    void VulkanRenderer::InitDevice()
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
            vkGetPhysicalDeviceMemoryProperties(m_GPU, &m_GPUMemoryProperties);
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

        float queuePriorities[]{1.0f};
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
        deviceCreateInfo.enabledLayerCount = m_DeviceLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = m_DeviceLayers.data();
        deviceCreateInfo.enabledExtensionCount = m_DeviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

        VkResult result = vkCreateDevice(m_GPU, &deviceCreateInfo, nullptr, &m_Device);
        VulkanDebug::CheckResult(result);

        // This is very minimal and simple setup
        // TODO: Read about all properties of all structures to create it more advanced way
        vkGetDeviceQueue(m_Device, m_GraphicsFamilyIndex, 0, &m_Queue);
    }

    void VulkanRenderer::DeinitDevice()
    {
        vkDestroyDevice(m_Device, nullptr);
    }


    VkInstance                    VulkanRenderer::GetVulkanInstance()                 const { return m_Instance; }
    VkPhysicalDevice              VulkanRenderer::GetVulkanPhysicalDevice()           const { return m_GPU; };
    VkDevice                      VulkanRenderer::GetVulkanDevice()                   const { return m_Device; }
    VkQueue                       VulkanRenderer::GetVulkanQueue()                    const { return m_Queue; }
    uint32_t                      VulkanRenderer::GetVulkanGraphicsFamilyIndex()      const { return m_GraphicsFamilyIndex; }
    const VkPhysicalDeviceProperties&   VulkanRenderer::GetVulkanPhysicalDeviceProperties() const { return m_GPUProperties; }
    const VkPhysicalDeviceMemoryProperties &VulkanRenderer::GetVulkanPhysicalDeviceMemoryProperties() const { return m_GPUMemoryProperties; }
}