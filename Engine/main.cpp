#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <deque>
#include <functional>
#include <tiny_obj_loader.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Window.hpp"
#include "VulkanHelper.hpp"
#include "Mesh.hpp"

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void PushDeleter(std::function<void()>&& function) 
    {
		deletors.push_back(function);
	}

	void Flush() 
    {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) 
        {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};

class VulkanRenderer
{
    private:
        VkInstance                  m_Instance          = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;
        VkPhysicalDevice            m_GPU               = VK_NULL_HANDLE;
        VkDevice                    m_Device            = VK_NULL_HANDLE;
        VkSurfaceKHR                m_Surface           = VK_NULL_HANDLE;

        VkSwapchainKHR              m_Swapchain             = VK_NULL_HANDLE;
        std::vector<VkImage>        m_SwapchainImages       = {};
        VkFormat                    m_SwapchainImageFormat  = {};
        VkExtent2D                  m_SwapchainExtent       = {};
        std::vector<VkImageView>    m_SwapchainImageViews   = {};

        VkQueue                     m_GraphicsQueue         = VK_NULL_HANDLE;
        uint32_t                    m_GraphicsQueueFamily   = 0; 
        VkQueue                     m_PresentQueue          = VK_NULL_HANDLE;
        uint32_t                    m_PresentQueueFamily    = 0;

        VkCommandPool               m_CommandPool           = VK_NULL_HANDLE;
        VkCommandBuffer             m_CommandBuffer         = VK_NULL_HANDLE;

        VkRenderPass                m_RenderPass            = VK_NULL_HANDLE;
        std::vector<VkFramebuffer>  m_Framebuffers          = {};
        
        DeletionQueue               m_DeletionQueue         = {};
        // render loop
        int                         m_FrameNumber           = 0;
        VkSemaphore                 m_PresentSemaphore      = VK_NULL_HANDLE;
        VkSemaphore                 m_RenderSemaphore       = VK_NULL_HANDLE;
        VkFence                     m_RenderFence           = VK_NULL_HANDLE;

        VkPipeline                  m_Pipeline              = VK_NULL_HANDLE;
        VkPipelineLayout            m_PipelineLayout        = VK_NULL_HANDLE;

        VkDeviceMemory              m_VertexBufferMemory    = VK_NULL_HANDLE;
        VkPipeline                  m_MeshPipeline          = VK_NULL_HANDLE;
        VkPipelineLayout            m_MeshPipelineLayout    = VK_NULL_HANDLE;
        Mesh                        m_Mesh                  = {};

        void InitInstance();
        void CreateSurface();
        void CreateDevice();
        void CreateSwapchain();
        void InitCommands();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();
        void DestroySyncObjects();
        void CreatePipeline();

        void LoadMeshes();
        void UploadMesh(Mesh& mesh);
    public:
        void InitVulkan();
        void DeinitVulkan();
        void Draw();
};

void VulkanRenderer::InitVulkan()
{
    InitInstance();
    CreateSurface();
    CreateDevice();
    CreateSwapchain();
    InitCommands();
    CreateRenderPass();
    CreateFramebuffers();
    CreateSyncObjects();
    CreatePipeline();
    LoadMeshes();
}

void VulkanRenderer::DeinitVulkan()
{
    vkDeviceWaitIdle(m_Device);

    m_DeletionQueue.Flush();
    vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkh::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::Draw()
{
    vkh::Check(vkWaitForFences(m_Device, 1, &m_RenderFence, true, 1000000000), "Wait for fences failed");
    vkh::Check(vkResetFences(m_Device, 1, &m_RenderFence), "Reset fence failed");

    uint32_t swapchainImageIndex;
    vkh::Check(vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, m_PresentSemaphore, nullptr, &swapchainImageIndex));

    // time to begin rendering commands
    vkh::Check(vkResetCommandBuffer(m_CommandBuffer, 0));

    VkCommandBufferBeginInfo cmdBeginCI
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo   = nullptr
    };

    vkh::Check(vkBeginCommandBuffer(m_CommandBuffer, &cmdBeginCI));

    VkClearValue clearValue;
	float flash = std::abs(std::sin(m_FrameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpBeginCI
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,

        .renderPass             = m_RenderPass,
        .framebuffer            = m_Framebuffers[swapchainImageIndex],

        //connect clear values
        .clearValueCount = 1,
        .pClearValues = &clearValue
    };

    rpBeginCI.renderArea.offset.x    = 0;
    rpBeginCI.renderArea.offset.y    = 0;
    rpBeginCI.renderArea.extent      = m_SwapchainExtent;

    vkCmdBeginRenderPass(m_CommandBuffer, &rpBeginCI, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline);

        //bind the mesh vertex buffer with offset 0
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &m_Mesh.vertexBuffer, &offset);
        // make a model view matrix for rendering the object
        // camera position
        glm::vec3 camPos = { 0.f,0.f,-2.f };

        glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        //camera projection
        glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Eternity::GetWindowWidth() / (float)Eternity::GetWindowHeight(), 0.1f, 200.0f);
        projection[1][1] *= -1;
        //model rotation
        glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(m_FrameNumber * 0.01f), glm::vec3(0, 1, 0));

        //calculate final mesh matrix
        glm::mat4 mesh_matrix = projection * view * model;

        MeshPushConstants constants;
        constants.renderMatrix = mesh_matrix;

        //upload the matrix to the GPU via pushconstants
        vkCmdPushConstants(m_CommandBuffer, m_MeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
        //we can now draw the mesh
        vkCmdDraw(m_CommandBuffer, m_Mesh.vertices.size(), 1, 0, 0);
    vkCmdEndRenderPass(m_CommandBuffer);

    vkh::Check(vkEndCommandBuffer(m_CommandBuffer));

    //prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit 
    {
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &m_PresentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &m_RenderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &m_CommandBuffer;

	//submit command buffer to the queue and execute it.
	// m_RenderFence will now block until the graphic commands finish execution
	vkh::Check(vkQueueSubmit(m_GraphicsQueue, 1, &submit, m_RenderFence), "Submit queue failed");

    // this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentI
    {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &m_RenderSemaphore,

        .swapchainCount     = 1,
        .pSwapchains        = &m_Swapchain,

        .pImageIndices = &swapchainImageIndex
    };

	vkh::Check(vkQueuePresentKHR(m_GraphicsQueue, &presentI));

	//increase the number of frames drawn
	m_FrameNumber++;
}

void VulkanRenderer::InitInstance()
{
    VkApplicationInfo appInfo
    {
        .sType          = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName    = "Eternity",
        .engineVersion  = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion     = VK_API_VERSION_1_2
    };

    auto inst_ret       = vkh::BuildInstance(appInfo, true);
    m_Instance          = inst_ret.first;
    m_DebugMessenger    = inst_ret.second;
}

void VulkanRenderer::CreateSurface()
{
    auto res = glfwCreateWindowSurface(m_Instance, Eternity::GetCurrentWindow(), nullptr, &m_Surface);
    vkh::Check(res, "Surface create failed");
}

void VulkanRenderer::CreateDevice()
{
    m_GPU       = vkh::SelectPhysicalDevice(m_Instance, m_Surface);
    m_Device    = vkh::BuildDevice(m_GPU, m_Surface);

    auto indices = vkh::FindQueueFamilies(m_GPU, m_Surface);
    m_GraphicsQueueFamily = indices.graphicsFamily.value();
    m_PresentQueueFamily = indices.presentFamily.value();
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);
}

void VulkanRenderer::CreateSwapchain()
{
    auto swachain_ret = vkh::BuildSwapchain(m_GPU, m_Surface, m_Device);
    m_Swapchain = swachain_ret.swapchain;

    m_SwapchainImages                   = swachain_ret.images;
    m_SwapchainImageFormat              = swachain_ret.imageFormat;
    m_SwapchainExtent                   = swachain_ret.extent;
    m_SwapchainImageViews               = swachain_ret.imageViews;

    m_DeletionQueue.PushDeleter([=]() 
    {
		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	});
}

void VulkanRenderer::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolCI
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        //we also want the pool to allow for resetting of individual command buffers
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        //the command pool will be one that can submit graphics commands
        .queueFamilyIndex = m_GraphicsQueueFamily
    };

	auto res = vkCreateCommandPool(m_Device, &commandPoolCI, nullptr, &m_CommandPool);
    vkh::Check(res, "Command pool create failed");

    VkCommandBufferAllocateInfo cmdAllocCI
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        //commands will be made from our commandPool
        .commandPool = m_CommandPool,
        // command level is Primary
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        //we will allocate 1 command buffer
        .commandBufferCount = 1
    };

    res = vkAllocateCommandBuffers(m_Device, &cmdAllocCI, &m_CommandBuffer);
    vkh::Check(res, "Command buffer allocate failed");

    m_DeletionQueue.PushDeleter([=]() 
    {
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
	});
}

void VulkanRenderer::CreateRenderPass()
{
    // the renderpass will use this color attachment.
	VkAttachmentDescription colorAttachment 
    {
	    // the attachment will have the format needed by the swapchain
	    .format         = m_SwapchainImageFormat,
	    // 1 sample, we won't be doing MSAA
	    .samples        = VK_SAMPLE_COUNT_1_BIT,
	    // Clear when this attachment is loaded
	    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    // keep the attachment stored when the renderpass ends
	    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
	    // don't care about stencil
	    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    // don't know or care about the starting layout of the attachment
	    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
	    // after the renderpass ends, the image has to be on a layout ready for display
	    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference coloAttachmentRef
    {
	    // attachment number will index into the pAttachments array in the parent renderpass itself
	    .attachment = 0,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

	// create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass
    {
        .pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount   = 1,
        .pColorAttachments      = &coloAttachmentRef
    };

    VkRenderPassCreateInfo renderPassCI
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        //connect the color attachment to the info
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        //connect the subpass to the info
        .subpassCount = 1,
        .pSubpasses = &subpass
    };
	
	auto res = vkCreateRenderPass(m_Device, &renderPassCI, nullptr, &m_RenderPass);
    vkh::Check(res, "RenderPass create failed");

    m_DeletionQueue.PushDeleter([=]() 
    {
		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    });
}

void VulkanRenderer::CreateFramebuffers()
{
    // create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo frambufferCI
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,

        .renderPass         = m_RenderPass,
        .attachmentCount    = 1,
        .width              = m_SwapchainExtent.width,
        .height             = m_SwapchainExtent.height,
        .layers             = 1
    };

	//grab how many images we have in the swapchain
	const uint32_t swapchainImageCount = m_SwapchainImages.size();
	m_Framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchainImageCount; i++) 
    {
		frambufferCI.pAttachments = &m_SwapchainImageViews[i];
		vkh::Check(vkCreateFramebuffer(m_Device, &frambufferCI, nullptr, &m_Framebuffers[i]), "Framebuffer create failed");

        m_DeletionQueue.PushDeleter([=]() 
        {
            vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
            vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
        });	
	}
}

void VulkanRenderer::CreateSyncObjects()
{
    // create syncronization objects

	VkFenceCreateInfo fenceCI
    {
    	.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        // we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
	    .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

	auto res = vkCreateFence(m_Device, &fenceCI, nullptr, &m_RenderFence);
    vkh::Check(res, "Fence create failed");
    m_DeletionQueue.PushDeleter([=]() 
    {
        vkDestroyFence(m_Device, m_RenderFence, nullptr);
    });
	//for the semaphores we don't need any flags
	VkSemaphoreCreateInfo semaphoreCI
    {
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	    .flags = 0
    };

	res = vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_PresentSemaphore);
    vkh::Check(res, "Present semaphore create failed");
	res = vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_RenderSemaphore);
    vkh::Check(res, "Render semaphore create failed");
    m_DeletionQueue.PushDeleter([=]() 
    {
        vkDestroySemaphore(m_Device, m_PresentSemaphore, nullptr);
        vkDestroySemaphore(m_Device, m_RenderSemaphore, nullptr);	
    });
}

void VulkanRenderer::DestroySyncObjects()
{
    vkDestroySemaphore(m_Device, m_RenderSemaphore, nullptr);
    vkDestroySemaphore(m_Device, m_PresentSemaphore, nullptr);
    vkDestroyFence(m_Device, m_RenderFence, nullptr);
}

void VulkanRenderer::CreatePipeline()
{
    VkShaderModule triangleVertShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/triangleVert.spv");
    VkShaderModule triangleFragShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/triangleFrag.spv");

    VkPipelineLayoutCreateInfo pipelineLayoutCI = vkh::PipelineLayoutCreateInfo();
    vkCreatePipelineLayout(m_Device, &pipelineLayoutCI, nullptr, &m_PipelineLayout);

    //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	vkh::PipelineBuilder pipelineBuilder;

	pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertShader));
	pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	pipelineBuilder.vertexInputInfo = vkh::VertexInputStateCreateInfo();
	
	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder.inputAssembly = vkh::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder.viewport.x          = 0.0f;
	pipelineBuilder.viewport.y          = 0.0f;
	pipelineBuilder.viewport.width      = static_cast<float>(m_SwapchainExtent.width);
	pipelineBuilder.viewport.height     = static_cast<float>(m_SwapchainExtent.height);
	pipelineBuilder.viewport.minDepth   = 0.0f;
	pipelineBuilder.viewport.maxDepth   = 1.0f;
	
	pipelineBuilder.scissor.offset = { 0, 0 };
	pipelineBuilder.scissor.extent = m_SwapchainExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder.rasterizer = vkh::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	//we don't use multisampling, so just run the default one
	pipelineBuilder.multisampling = vkh::MultisamplingStateCreateInfo();
	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder.colorBlendAttachment = vkh::ColorBlendAttachmentState();		
	//use the triangle layout we created
	pipelineBuilder.pipelineLayout = m_PipelineLayout;
    //finally build the pipeline
	m_Pipeline = pipelineBuilder.BuildPipeline(m_Device, m_RenderPass);

    //build the mesh pipeline
	VertexInputDescription vertexDescription = Vertex::GetVertexDescription();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

    // we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo meshPipelineLayoutCI = vkh::PipelineLayoutCreateInfo();
    
	// setup push constants
	VkPushConstantRange pushConstant {};
	// this push constant range starts at the beginning
	pushConstant.offset     = 0;
	// this push constant range takes up the size of a MeshPushConstants struct
	pushConstant.size       = sizeof(MeshPushConstants);
	// this push constant range is accessible only in the vertex shader
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	meshPipelineLayoutCI.pPushConstantRanges       = &pushConstant;
	meshPipelineLayoutCI.pushConstantRangeCount    = 1;

    auto res = vkCreatePipelineLayout(m_Device, &meshPipelineLayoutCI, nullptr, &m_MeshPipelineLayout);
    vkh::Check(res, "PipelineLayot create failed");

	//clear the shader stages for the builder
	pipelineBuilder.shaderStages.clear();

    VkShaderModule meshVertShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/tri_mesh.spv");

    pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
    pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));
    pipelineBuilder.pipelineLayout = m_MeshPipelineLayout;
    m_MeshPipeline = pipelineBuilder.BuildPipeline(m_Device, m_RenderPass);

    vkDestroyShaderModule(m_Device, meshVertShader, nullptr);
    vkDestroyShaderModule(m_Device, triangleFragShader, nullptr);
    vkDestroyShaderModule(m_Device, triangleVertShader, nullptr);

    m_DeletionQueue.PushDeleter([=]() 
    {
        vkDestroyPipeline(m_Device, m_MeshPipeline, nullptr);
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        vkDestroyPipelineLayout(m_Device, m_MeshPipelineLayout, nullptr);
    });
}

uint32_t FindMemoryType(const VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanRenderer::UploadMesh(Mesh& mesh)
{
    VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//this is the total size, in bytes, of the buffer we are allocating
	bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
	//this buffer is going to be used as a Vertex Buffer
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_Device, &bufferInfo, nullptr, &mesh.vertexBuffer);


    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, mesh.vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(m_GPU, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto res = vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_VertexBufferMemory);
    vkh::Check(res, "Memory allocate failed");
    vkBindBufferMemory(m_Device, mesh.vertexBuffer, m_VertexBufferMemory, 0);

    void* data;
    vkMapMemory(m_Device, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);
    std::memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vkUnmapMemory(m_Device, m_VertexBufferMemory);

    m_DeletionQueue.PushDeleter([=]()
    {
        vkDestroyBuffer(m_Device, mesh.vertexBuffer, nullptr);
    });
}

void VulkanRenderer::LoadMeshes()
{
    // m_Mesh.vertices.resize(3);

	// //vertex positions
	// m_Mesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	// m_Mesh.vertices[1].position = {-1.f, 1.f, 0.0f };
	// m_Mesh.vertices[2].position = { 0.f,-1.f, 0.0f };

	// //vertex colors, all green
	// m_Mesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	// m_Mesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	// m_Mesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
	
	//we don't care about the vertex normals
    m_Mesh.LoadFromOBJ("../Engine/assets/monkey.obj");
	UploadMesh(m_Mesh);
}

int main(int, char **) 
{
    Eternity::CreateWindow(800, 600, "Eternity");

    VulkanRenderer renderer;
    renderer.InitVulkan();

    while (!glfwWindowShouldClose(Eternity::GetCurrentWindow()))
    {
        renderer.Draw();
        glfwPollEvents();
    }

    renderer.DeinitVulkan();

    Eternity::DestroyWindow();
    return 0; 
}