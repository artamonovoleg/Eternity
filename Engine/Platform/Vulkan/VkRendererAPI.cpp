////
//// Created by artamonovoleg on 20.12.2020.
////
//
//#include "VkRendererAPI.hpp"
//
//namespace Eternity
//{
//    VkRendererAPI::VkRendererAPI()
//    {
//        SetupDebug();
//        SetupLayersAndExtensions();
//        InitInstance();
//        InitDebug();
//        InitDevice();
//    }
//
//    VkRendererAPI::~VkRendererAPI()
//    {
//        DeinitDevice();
//        DeinitDebug();
//        DeinitInstance();
//    }
//
//    void VkRendererAPI::InitInstance()
//    {
//        // TODO: think about move it from here its not good idea.
//        VkApplicationInfo applicationInfo{};
//        applicationInfo.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//        applicationInfo.apiVersion                  = VK_MAKE_VERSION(1, 0 ,2);
//        applicationInfo.engineVersion               = VK_MAKE_VERSION(0, 0, 4);
//        applicationInfo.pEngineName                 = "Eternity";
//
//        VkInstanceCreateInfo instanceCreateInfo{};
//        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//        instanceCreateInfo.pApplicationInfo         = &applicationInfo;
//        // debug options
//        instanceCreateInfo.enabledLayerCount        = m_InstanceLayers.size();
//        instanceCreateInfo.ppEnabledLayerNames      = m_InstanceLayers.data();
//        instanceCreateInfo.enabledExtensionCount    = m_InstanceExtensions.size();
//        instanceCreateInfo.ppEnabledExtensionNames  = m_InstanceExtensions.data();
//        instanceCreateInfo.pNext                    = &m_DebugReportCallbackCreateInfo;
//
//        // TODO: Read about other parameters of this structure
//        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
//
//        VkAssert(result);
//    }
//
//    void VkRendererAPI::DeinitInstance()
//    {
//        vkDestroyInstance(m_Instance, nullptr);
//    }
//
//    // needed to create surface
//    void VkRendererAPI::SetupLayersAndExtensions()
//    {
//        uint32_t extensionsCount				= 0;
//        const char ** instanceExtensionsBuffer		= glfwGetRequiredInstanceExtensions(&extensionsCount);
//        for(uint32_t i = 0; i < extensionsCount; i++)
//            m_InstanceExtensions.push_back(instanceExtensionsBuffer[i]);
//
//        m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
//    }
//
//    // Create physical and logical device
//    // find gpu and create logical device to manipulate this
//    void VkRendererAPI::InitDevice()
//    {
//        // TODO: Generally its not good idea to store it in one method. its better to decompose it
//        {
//            uint32_t gpuCount = 0; // count physical devices
//            vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);
//            // after that we have amount of gpu's
//            std::vector<VkPhysicalDevice> gpuList(gpuCount);
//            // call again to save gpu handles
//            vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpuList.data());
//            // TODO: select the best, not take first gpu from list
//            m_GPU = gpuList.at(0);
//            vkGetPhysicalDeviceProperties(m_GPU, &m_GPUProperties);
//        }
//        {
//            // describe queue families that we will use
//            uint32_t familyCount = 0;
//            vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, nullptr);
//            std::vector<VkQueueFamilyProperties> familyPropertiesList{familyCount};
//            vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, familyPropertiesList.data());
//
//            bool found = false;
//            for (uint32_t i = 0; i < familyCount; i++)
//            {
//                if (familyPropertiesList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//                {
//                    found = true;
//                    m_GraphicsFamilyIndex = i;
//                }
//            }
//            ET_CORE_ASSERT(found, "Queue family supporting graphics not found");
//        }
//        {
//            // list all instance layers
//            // its debug function
//            uint32_t layerCount = 0;
//            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//            std::vector<VkLayerProperties> layerPropertyList(layerCount);
//            vkEnumerateInstanceLayerProperties(&layerCount, layerPropertyList.data());
//            // TODO: change to Logging
//            std::cout << "Instance layer available: \n";
//            for (const auto& layer : layerPropertyList)
//                std::cout << "\t" << layer.layerName << "  |  " << layer.description << std::endl;
//        }
//        {
//            // list all device layers
//            // its debug function
//            uint32_t layerCount = 0;
//            vkEnumerateDeviceLayerProperties(m_GPU, &layerCount, nullptr);
//            std::vector<VkLayerProperties> layerPropertyList(layerCount);
//            vkEnumerateDeviceLayerProperties(m_GPU, &layerCount, layerPropertyList.data());
//            // TODO: change to Logging
//            std::cout << "Device layer available: \n";
//            for (const auto& layer : layerPropertyList)
//                std::cout << "\t" << layer.layerName << "  |  " << layer.description << std::endl;
//        }
//        float queuePriorities[] { 1.0f };
//        VkDeviceQueueCreateInfo queueCreateInfo{};
//        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//        queueCreateInfo.queueFamilyIndex = m_GraphicsFamilyIndex;
//        queueCreateInfo.queueCount = 1;
//        queueCreateInfo.pQueuePriorities = queuePriorities;
//
//        VkDeviceCreateInfo deviceCreateInfo{};
//        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//        deviceCreateInfo.queueCreateInfoCount = 1;
//        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
//        // debug options
//        deviceCreateInfo.enabledLayerCount        = m_DeviceLayers.size();
//        deviceCreateInfo.ppEnabledLayerNames      = m_DeviceLayers.data();
//        deviceCreateInfo.enabledExtensionCount    = m_DeviceExtensions.size();
//        deviceCreateInfo.ppEnabledExtensionNames  = m_DeviceExtensions.data();
//
//        VkResult result = vkCreateDevice(m_GPU, &deviceCreateInfo, nullptr, &m_Device);
//        VkAssert(result);
//
//        // This is very minimal and simple setup
//        // TODO: Read about all properties of all structures to create it more advanced way
//        vkGetDeviceQueue(m_Device, m_GraphicsFamilyIndex, 0, &m_Queue);
//    }
//
//    void VkRendererAPI::DeinitDevice()
//    {
//        vkDestroyDevice(m_Device, nullptr);
//    }
//
//    #if BUILD_ENABLE_VULKAN_DEBUG
//
//    VKAPI_ATTR VkBool32 VKAPI_CALL
//    VulkanDebugCallback(VkDebugReportFlagsEXT       flags,
//                        VkDebugReportObjectTypeEXT  objType,
//                        uint64_t                    sourceObjs,
//                        size_t                      location,
//                        int32_t                     msgCode,
//                        const char*                 layerPrefix,
//                        const char*                 msg,
//                        void*                       userData)
//    {
//        // TODO: change to logging
//        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
//            std::cout << "INFO: ";
//        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
//            std::cout << "WARN: ";
//        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//            std::cout << "PERFORMANCE: ";
//        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//            std::cout << "ERROR: ";
//        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//            std::cout << "DEBUG: ";
//        std::cout << " [" << layerPrefix << "] ";
//        std::cout << msg << std::endl;
//        return VK_FALSE;
//    }
//
//    void VkRendererAPI::SetupDebug()
//    {
//        m_DebugReportCallbackCreateInfo.sType         = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
//        m_DebugReportCallbackCreateInfo.pfnCallback   = VulkanDebugCallback;
//        m_DebugReportCallbackCreateInfo.flags         = VK_DEBUG_REPORT_INFORMATION_BIT_EXT         |
//                                                        VK_DEBUG_REPORT_WARNING_BIT_EXT             |
//                                                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
//                                                        VK_DEBUG_REPORT_ERROR_BIT_EXT               |
//                                                        VK_DEBUG_REPORT_DEBUG_BIT_EXT               |
//                                                        0;
//
//        m_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
//        m_InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//        m_DeviceLayers.push_back("VK_LAYER_KHRONOS_validation");
//    }
//
//    PFN_vkCreateDebugReportCallbackEXT fVkCreateDebugReportCallbackEXT      = VK_NULL_HANDLE;
//    PFN_vkDestroyDebugReportCallbackEXT fVkDestroyDebugReportCallbackEXT    = VK_NULL_HANDLE;
//
//    void VkRendererAPI::InitDebug()
//    {
//        fVkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
//        fVkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
//        ET_CORE_ASSERT(fVkCreateDebugReportCallbackEXT != VK_NULL_HANDLE &&
//                        fVkDestroyDebugReportCallbackEXT != VK_NULL_HANDLE, "Can't fetch debug function pointers");
//        fVkCreateDebugReportCallbackEXT(m_Instance, &m_DebugReportCallbackCreateInfo, nullptr, &m_DebugReport);
//    }
//
//    void VkRendererAPI::DeinitDebug()
//    {
//        fVkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, nullptr);
//    }
//
//    #else
//    void VkRendererAPI::SetupDebug() {}
//    void VkRendererAPI::InitDebug() {}
//    void VkRendererAPI::DeinitDebug() {}
//    #endif // BUILD_ENABLE_VULKAN_DEBUG
//
//
//    VkInstance                    VkRendererAPI::GetVulkanInstance()                 const { return m_Instance; }
//    VkPhysicalDevice              VkRendererAPI::GetVulkanPhysicalDevice()           const { return m_GPU; };
//    VkDevice                      VkRendererAPI::GetVulkanDevice()                   const { return m_Device; }
//    VkQueue                       VkRendererAPI::GetVulkanQueue()                    const { return m_Queue; }
//    uint32_t                      VkRendererAPI::GetVulkanGraphicsFamilyIndex()      const { return m_GraphicsFamilyIndex; }
//    const VkPhysicalDeviceProperties&   VkRendererAPI::GetVulkanPhysicalDeviceProperties() const { return m_GPUProperties; }
//}