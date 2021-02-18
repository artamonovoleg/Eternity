#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/hash.hpp>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <list>
#include <unordered_map>
#include "Utils.hpp"
#include "Window.hpp"
#include "EventSystem.hpp"
#include "Input.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "Image.hpp"
#include "DepthImage.hpp"
#include "Image2D.hpp"
#include "Framebuffers.hpp"
#include "CommandPool.hpp"
#include "Buffer.hpp"
#include "UniformBuffer.hpp"
#include "CommandBuffer.hpp"
#include "Shader.hpp"
#include "Descriptors.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSets.hpp"
#include "GraphicsPipelineLayout.hpp"
#include "GraphicsPipeline.hpp"

#include "VulkanApp.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

using namespace Eternity;

int main() 
{
    Eternity::CreateWindow(800, 600, "Eternity");
    Eternity::EventSystem::Init();
    Eternity::Input::Init();
    Eternity::Input::SetMouseMode(Eternity::Input::MouseMode::Capture);

    Eternity::Renderable model;

    auto& vertices = model.vertices;
    // front
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });
    // right
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });

    // back
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });

    // left
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 0) });

    // up
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(1.0f / 16.0f, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) });

    // down
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });

    auto& indices = model.indices;
    indices = std::vector<uint32_t> { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };

    Eternity::VulkanApp app;
    
    size_t bind;

    std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    app.SetRenderCamera(camera);

    while (!Eternity::WindowShouldClose()) 
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera->Update(deltaTime);

        if (Input::GetKeyDown(Key::C))
        {
            app.LoadModel(model, bind);
            for (auto& i : vertices)
                i.pos.x += 1.0f;
        }
        
        if (Input::GetKeyDown(Key::X))
        {
            for (auto& i : vertices)
                i.pos.x -= 1.0f;
            app.UnloadModel(bind);
            bind--;
        }

        EventSystem::PollEvents();
        app.DrawFrame();
    }

    Eternity::DestroyWindow();

    return EXIT_SUCCESS;
}