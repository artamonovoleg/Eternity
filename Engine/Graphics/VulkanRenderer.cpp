#include <cstring>
#include <tiny_obj_loader.h>
#include "VulkanRenderer.hpp"
#include "VulkanDebugCallback.hpp"
#include "VulkanHelper.hpp"


float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

namespace Eternity
{
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
        InitScene();
    }

    void VulkanRenderer::DeinitVulkan()
    {
        vkDeviceWaitIdle(m_Device);
        m_DeletionQueue.Flush();
    }

    void VulkanRenderer::Draw()
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        m_Camera.Update(deltaTime);
        
        vkh::Check(vkWaitForFences(m_Device, 1, &GetCurrentFrame().renderFence, true, UINT64_MAX), "Wait for fences failed");
        vkh::Check(vkResetFences(m_Device, 1, &GetCurrentFrame().renderFence), "Reset fence failed");

        uint32_t swapchainImageIndex;
        vkh::Check(vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_PresentSemaphore, nullptr, &swapchainImageIndex));

        // time to begin rendering commands
        vkh::Check(vkResetCommandBuffer(GetCurrentFrame().commandBuffer, 0));

        VkCommandBufferBeginInfo cmdBeginCI
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo   = nullptr
        };

        vkh::Check(vkBeginCommandBuffer(GetCurrentFrame().commandBuffer, &cmdBeginCI));

        VkClearValue clearValue;
        float flash = std::abs(std::sin(m_FrameNumber / 120.f));
        clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        //clear depth at 1
        VkClearValue depthClear;
        depthClear.depthStencil.depth = 1.f;

        //start the main renderpass. 
        //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
        VkRenderPassBeginInfo rpBeginCI
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,

            .renderPass             = m_RenderPass,
            .framebuffer            = m_Framebuffers[swapchainImageIndex],

            //connect clear values
            .clearValueCount = 2,
        };

        VkClearValue clearValues[] = { clearValue, depthClear };
        rpBeginCI.pClearValues = &clearValues[0];

        rpBeginCI.renderArea.offset.x    = 0;
        rpBeginCI.renderArea.offset.y    = 0;
        rpBeginCI.renderArea.extent      = m_SwapchainExtent;

        vkCmdBeginRenderPass(GetCurrentFrame().commandBuffer, &rpBeginCI, VK_SUBPASS_CONTENTS_INLINE);
            // vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline);

            // //bind the mesh vertex buffer with offset 0
            // VkDeviceSize offset = 0;
            // vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &m_Mesh.vertexBuffer, &offset);
            // // make a model view matrix for rendering the object
            // // camera position
            // glm::mat4 view = m_Camera.GetViewMatrix();
            // //camera projection
            // glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Eternity::GetWindowWidth() / (float)Eternity::GetWindowHeight(), 0.1f, 200.0f);
            // projection[1][1] *= -1;
            // //model rotation
            // glm::mat4 model = glm::mat4(1.0f);

            // //calculate final mesh matrix
            // glm::mat4 mesh_matrix = projection * view * model;

            // MeshPushConstants constants;
            // constants.renderMatrix = mesh_matrix;

            // //upload the matrix to the GPU via pushconstants
            // vkCmdPushConstants(m_CommandBuffer, m_MeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
            // //we can now draw the mesh
            // vkCmdDraw(m_CommandBuffer, m_Mesh.vertices.size(), 1, 0, 0);
            DrawObjects(GetCurrentFrame().commandBuffer, m_Renderables, m_Renderables.size());
        vkCmdEndRenderPass(GetCurrentFrame().commandBuffer);

        vkh::Check(vkEndCommandBuffer(GetCurrentFrame().commandBuffer));

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
        submit.pCommandBuffers = &GetCurrentFrame().commandBuffer;

        //submit command buffer to the queue and execute it.
        // m_RenderFence will now block until the graphic commands finish execution
        vkh::Check(vkQueueSubmit(m_GraphicsQueue, 1, &submit, GetCurrentFrame().renderFence), "Submit queue failed");

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

        auto inst_ret       = vkh::BuildInstance(appInfo, true, Eternity::DebugCallback);
        m_Instance          = inst_ret.first;
        m_DebugMessenger    = inst_ret.second;

        m_DeletionQueue.PushDeleter([=]()
        {
            vkh::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            vkDestroyInstance(m_Instance, nullptr);
        });
    }

    void VulkanRenderer::CreateSurface()
    {
        auto res = glfwCreateWindowSurface(m_Instance, Eternity::GetCurrentWindow(), nullptr, &m_Surface);
        vkh::Check(res, "Surface create failed");
        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        });
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

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyDevice(m_Device, nullptr);
        });
    }

    void VulkanRenderer::CreateSwapchain()
    {
        auto swachain_ret = vkh::BuildSwapchain(Eternity::GetCurrentWindow(), m_GPU, m_Surface, m_Device);
        m_Swapchain = swachain_ret.swapchain;

        m_SwapchainImages                   = swachain_ret.images;
        m_SwapchainImageFormat              = swachain_ret.imageFormat;
        m_SwapchainExtent                   = swachain_ret.extent;
        m_SwapchainImageViews               = swachain_ret.imageViews;

        VkExtent3D depthImageExtent
        {
            .width      = m_SwapchainExtent.width,
            .height     = m_SwapchainExtent.height,
            .depth      = 1
        };

        m_DepthFormat = VK_FORMAT_D32_SFLOAT;
        VkImageCreateInfo dimgCI = vkh::ImageCreateInfo(depthImageExtent, m_DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        m_DepthImage = vkh::CreateImage(m_Device, m_GPU, dimgCI, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImageMemory);

        VkImageViewCreateInfo dviewCI = vkh::ImageViewCreateInfo(m_DepthFormat, m_DepthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
        auto res = vkCreateImageView(m_Device, &dviewCI, nullptr, &m_DepthImageView);
        vkh::Check(res, "Image view create failed");

        m_DeletionQueue.PushDeleter([=]() 
        {   
            vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);
            vkDestroyImage(m_Device, m_DepthImage, nullptr);
            vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
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

        for (int i = 0; i < FRAME_OVERLAP; i++) 
        {
            auto res = vkCreateCommandPool(m_Device, &commandPoolCI, nullptr, &m_Frames[i].commandPool);
            vkh::Check(res, "Command pool create failed");

            VkCommandBufferAllocateInfo cmdAllocCI
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                //commands will be made from our commandPool
                .commandPool        = m_Frames[i].commandPool,
                // command level is Primary
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                //we will allocate 1 command buffer
                .commandBufferCount = 1
            };

            //allocate the default command buffer that we will use for rendering
            res = vkAllocateCommandBuffers(m_Device, &cmdAllocCI, &m_Frames[i].commandBuffer);
            vkh::Check(res, "Command buffer allocate failed");

            m_DeletionQueue.PushDeleter([=]() 
            {
                vkDestroyCommandPool(m_Device, m_Frames[i].commandPool, nullptr);
            });
	    }

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

        VkAttachmentDescription depthAttachment = {};
        // Depth attachment
        depthAttachment.flags = 0;
        depthAttachment.format = m_DepthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // create 1 subpass, which is the minimum you can do
        VkSubpassDescription subpass
        {
            .pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount   = 1,
            .pColorAttachments      = &coloAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef
        };

        VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassCI
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            //connect the color attachment to the info
            .attachmentCount = 2,
            .pAttachments = &attachments[0],
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
            VkImageView attachments[2];
            attachments[0] = m_SwapchainImageViews[i];
            attachments[1] = m_DepthImageView;

            frambufferCI.pAttachments       = attachments;
            frambufferCI.attachmentCount    = 2;

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

        VkSemaphoreCreateInfo semaphoreCI
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .flags = 0
        };

        auto res = vkCreateFence(m_Device, &fenceCI, nullptr, &m_RenderFence);
        vkh::Check(res, "Fence create failed");
        m_DeletionQueue.PushDeleter([=]() 
        {
            vkDestroyFence(m_Device, m_RenderFence, nullptr);
        });
        //for the semaphores we don't need any flags


        res = vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_PresentSemaphore);
        vkh::Check(res, "Present semaphore create failed");
        res = vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_RenderSemaphore);
        vkh::Check(res, "Render semaphore create failed");
        m_DeletionQueue.PushDeleter([=]() 
        {
            vkDestroySemaphore(m_Device, m_PresentSemaphore, nullptr);
            vkDestroySemaphore(m_Device, m_RenderSemaphore, nullptr);	
        });


        for (int i = 0; i < FRAME_OVERLAP; i++) 
        {     
            vkh::Check(vkCreateFence(m_Device, &fenceCI, nullptr, &m_Frames[i].renderFence));

            //enqueue the destruction of the fence
            m_DeletionQueue.PushDeleter([=]() 
            {
                vkDestroyFence(m_Device, m_Frames[i].renderFence, nullptr);
            });

            vkh::Check(vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_Frames[i].presentSemaphore));
            vkh::Check(vkCreateSemaphore(m_Device, &semaphoreCI, nullptr, &m_Frames[i].renderSemaphore));

            //enqueue the destruction of semaphores
            m_DeletionQueue.PushDeleter([=]() 
            {
                vkDestroySemaphore(m_Device, m_Frames[i].presentSemaphore, nullptr);
                vkDestroySemaphore(m_Device, m_Frames[i].renderSemaphore, nullptr);
            });
	    }
    }

    void VulkanRenderer::DestroySyncObjects()
    {
        vkDestroySemaphore(m_Device, m_RenderSemaphore, nullptr);
        vkDestroySemaphore(m_Device, m_PresentSemaphore, nullptr);
        vkDestroyFence(m_Device, m_RenderFence, nullptr);
    }

    void VulkanRenderer::CreatePipeline()
    {
        VkShaderModule meshVertShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/meshVert.spv");
        VkShaderModule meshFragShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/meshFrag.spv");

        // build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
        vkh::PipelineBuilder pipelineBuilder;

        pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
        pipelineBuilder.shaderStages.push_back(vkh::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, meshFragShader));

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
        pipelineBuilder.rasterizer              = vkh::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
        //we don't use multisampling, so just run the default one
        pipelineBuilder.multisampling           = vkh::MultisamplingStateCreateInfo();
        //a single blend attachment with no blending and writing to RGBA
        pipelineBuilder.colorBlendAttachment    = vkh::ColorBlendAttachmentState();		
        //use the triangle layout we created
        pipelineBuilder.pipelineLayout          = m_PipelineLayout;
        //finally build the pipeline
        pipelineBuilder.depthStencil            = vkh::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

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

        pipelineBuilder.pipelineLayout = m_MeshPipelineLayout;
        pipelineBuilder.depthStencil    = vkh::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

        m_MeshPipeline = pipelineBuilder.BuildPipeline(m_Device, m_RenderPass);

        CreateMaterial(m_MeshPipeline, m_MeshPipelineLayout, "defaultmesh");

        vkDestroyShaderModule(m_Device, meshVertShader, nullptr);
        vkDestroyShaderModule(m_Device, meshFragShader, nullptr);

        m_DeletionQueue.PushDeleter([=]() 
        {
            vkDestroyPipeline(m_Device, m_MeshPipeline, nullptr);
            vkDestroyPipelineLayout(m_Device, m_MeshPipelineLayout, nullptr);
        });
    }

    void VulkanRenderer::UploadMesh(Mesh& mesh)
    {
        mesh.vertexBuffer = vkh::CreateVertexBuffer(m_Device, m_GPU, mesh.vertices.size() * sizeof(Vertex), m_VertexBufferMemory);

        void* data;
        vkMapMemory(m_Device, m_VertexBufferMemory, 0, mesh.vertices.size() * sizeof(Vertex), 0, &data);
        std::memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
        vkUnmapMemory(m_Device, m_VertexBufferMemory);

        m_DeletionQueue.PushDeleter([=, &mesh]()
        {
            vkDestroyBuffer(m_Device, mesh.vertexBuffer, nullptr);
            vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
        });
    }

    void VulkanRenderer::LoadMeshes()
    {
        m_Mesh.LoadFromOBJ("../Engine/assets/monkey.obj");
        UploadMesh(m_Mesh);
        m_Meshes["monkey"] = m_Mesh;
    }

    std::shared_ptr<Material> VulkanRenderer::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
    {
        Material mat;
        mat.pipeline = pipeline;
        mat.pipelineLayout = layout;
        m_Materials[name] = mat;
        return std::make_shared<Material>(m_Materials[name]);
    }

    std::shared_ptr<Material> VulkanRenderer::GetMaterial(const std::string& name)
    {
        //search for the object, and return nullpointer if not found
        auto it = m_Materials.find(name);
        if (it == m_Materials.end()) 
            return nullptr;
        else 
            return std::make_shared<Material>((*it).second);
    }

    std::shared_ptr<Mesh> VulkanRenderer::GetMesh(const std::string& name)
    {
        auto it = m_Meshes.find(name);
        if (it == m_Meshes.end()) 
            return nullptr;
        else 
            return std::make_shared<Mesh>((*it).second);
    }

    void VulkanRenderer::DrawObjects(VkCommandBuffer cmd, std::vector<RenderObject>& first, int count)
    {
        glm::mat4 view = m_Camera.GetViewMatrix();
        //camera projection
        glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Eternity::GetWindowWidth() / (float)Eternity::GetWindowHeight(), 0.1f, 200.0f);
        projection[1][1] *= -1;

        std::shared_ptr<Mesh>       lastMesh = nullptr;
        std::shared_ptr<Material>   lastMaterial = nullptr;

        for (int i = 0; i < count; i++)
        {
            RenderObject& object = first[i];

            //only bind the pipeline if it doesnt match with the already bound one
            if (object.material != lastMaterial) 
            {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
                lastMaterial = object.material;
            }

            glm::mat4 model = object.transformMatrix;
            //final render matrix, that we are calculating on the cpu
            glm::mat4 meshMatrix = projection * m_Camera.GetViewMatrix() * model;

            MeshPushConstants constants;
            constants.renderMatrix = meshMatrix;

            //upload the mesh to the GPU via pushconstants
            vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

            //only bind the mesh if it's a different one from last bind
            if (object.mesh != lastMesh) 
            {
                //bind the mesh vertex buffer with offset 0
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer, &offset);
                lastMesh = object.mesh;
            }

            //we can now draw
            vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
        }
    }

    void VulkanRenderer::InitScene()
    {
        RenderObject monkey{};
        monkey.mesh             = GetMesh("monkey");
        monkey.material         = GetMaterial("defaultmesh");
        monkey.transformMatrix  = glm::mat4(1.0f);

        m_Renderables.push_back(monkey);
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(3, 0, 0));
        monkey.transformMatrix = translation;
        m_Renderables.push_back(monkey);
        translation = glm::translate(glm::mat4(1.0f), glm::vec3(6, 0, 0));
        monkey.transformMatrix = translation;
        m_Renderables.push_back(monkey);
    }

    FrameData& VulkanRenderer::GetCurrentFrame()
    {
        return m_Frames[m_FrameNumber % FRAME_OVERLAP];
    }
}