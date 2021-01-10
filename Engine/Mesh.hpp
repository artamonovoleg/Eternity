#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

struct VertexInputDescription 
{
	std::vector<VkVertexInputBindingDescription>    bindings;
	std::vector<VkVertexInputAttributeDescription>  attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex 
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription GetVertexDescription();
};

struct MeshPushConstants 
{
	glm::vec4 data;
	glm::mat4 renderMatrix;
};

struct Mesh 
{
	std::vector<Vertex> vertices        = {};

	VkBuffer            vertexBuffer    = VK_NULL_HANDLE;

	bool 				LoadFromOBJ(const std::string& filename);
};

