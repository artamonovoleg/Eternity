#include "Eternity.hpp"
#include "./Sandbox/Chunk.hpp"
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
    Eternity::VulkanApp app;
	

    std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    app.SetRenderCamera(camera);

    Chunk chunk(glm::vec3(0, 0, 0));

    app.LoadModel(chunk);

    while (!Eternity::WindowShouldClose()) 
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera->Update(deltaTime);

        if (Eternity::Input::GetKeyDown(Key::X))
        {
            chunk.TestBlockDig();
            app.UnloadModel(chunk);
            app.LoadModel(chunk);
        }

        EventSystem::PollEvents();
        app.DrawFrame();
    }

    Eternity::DestroyWindow();

    return EXIT_SUCCESS;
}