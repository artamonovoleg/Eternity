#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vkh
{
    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() 
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    struct Swapchain
    {
        VkSwapchainKHR              swapchain   = VK_NULL_HANDLE;
        std::vector<VkImage>        images      = {};
        VkFormat                    imageFormat = {};
        VkExtent2D                  extent      = {};
        std::vector<VkImageView>    imageViews  = {};
    };

    bool                                            IsVulkanDebugEnabled();

    void                                            Check(VkResult result, const std::string& msg = "");
    const std::vector<const char*>&                 GetValidationLayers();
    std::vector<const char*>                        GetRequiredExtensions();

    VkResult                                        CreateDebugUtilsMessengerEXT
    (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void                                            DestroyDebugUtilsMessengerEXT
    (VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    /// Create instance funtion
    /// params: appInfo
    //          debugEnabled - VkDebugUtilsMessengerEXT creates only if true, if false = VK_NULL_HANDLE
    std::pair<VkInstance, VkDebugUtilsMessengerEXT> BuildInstance(const VkApplicationInfo& appInfo, bool debugEnabled);

    QueueFamilyIndices                              FindQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    VkPhysicalDevice                                SelectPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
    VkDevice                                        BuildDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

    VkSurfaceFormatKHR                              ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR                                ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D                                      ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapchainSupportDetails                         QuerySwapchainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    Swapchain                                       BuildSwapchain(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const VkDevice& device);

    VkShaderModule                                  CreateShaderModule(const VkDevice& device, const std::string& filename);

    uint32_t FindMemoryType(const VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    class PipelineBuilder 
    {
        public:
            std::vector<VkPipelineShaderStageCreateInfo>    shaderStages;
            VkPipelineVertexInputStateCreateInfo            vertexInputInfo;
            VkPipelineInputAssemblyStateCreateInfo          inputAssembly;
            VkViewport                                      viewport;
            VkRect2D                                        scissor;
            VkPipelineRasterizationStateCreateInfo          rasterizer;
            VkPipelineColorBlendAttachmentState             colorBlendAttachment;
            VkPipelineMultisampleStateCreateInfo            multisampling;
            VkPipelineLayout                                pipelineLayout;
            VkPipelineDepthStencilStateCreateInfo           depthStencil;

            VkPipeline                              BuildPipeline(const VkDevice& device, const VkRenderPass& pass);
    };

    VkBuffer                                        CreateVertexBuffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, uint32_t size, VkDeviceMemory& memory);

    VkPipelineShaderStageCreateInfo                 PipelineShaderStageCreateInfo(const VkShaderStageFlagBits& stage, const VkShaderModule& shaderModule);
    VkPipelineVertexInputStateCreateInfo            VertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo          InputAssemblyCreateInfo(const VkPrimitiveTopology& topology);
    VkPipelineRasterizationStateCreateInfo          RasterizationStateCreateInfo(const VkPolygonMode& polygonMode);
    VkPipelineMultisampleStateCreateInfo            MultisamplingStateCreateInfo();
    VkPipelineColorBlendAttachmentState             ColorBlendAttachmentState();
    VkPipelineLayoutCreateInfo                      PipelineLayoutCreateInfo();

    VkImageCreateInfo                               ImageCreateInfo(const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
    VkImage                                         CreateImage(VkDevice& device, VkPhysicalDevice& physicalDevice, VkImageCreateInfo& imageCI, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
    VkImageViewCreateInfo                           ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
    VkPipelineDepthStencilStateCreateInfo           DepthStencilCreateInfo(VkBool32 bDepthTest, VkBool32 bDepthWrite, VkCompareOp compareOp);
}