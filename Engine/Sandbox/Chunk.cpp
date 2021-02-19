#include "Chunk.hpp"
#include "Block.hpp"
///
    std::vector<Vertex> block { 
            { .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) },
            { .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) },
    // right
            { .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) },
            { .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) },

    // back
            { .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) },
            { .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) },

    // left
            { .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) },
            { .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) },

    // up
            { .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) },
            { .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(1.0f / 16.0f, 0) },
            { .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) },

    // down
            { .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(2.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) },
            { .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(2.0f / 16.0f, 0.0f) },
            { .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) } };

    std::vector<uint32_t> inds { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };
///

Chunk::Chunk(glm::ivec3 pos)
    : m_Pos(pos)
{
	GenerateBlocks();
}

void Chunk::GenerateBlocks()
{
    for (int z = 0; z < m_Size; z++)
    {
        for (int y = 0; y < m_Size; y++)
        {
            for (int x = 0; x < m_Size; x++)
            {
                for (const auto& vert : block)
                    vertices.push_back(Vertex{ vert.pos + glm::vec3(x, y, z), vert.texCoord });
                indices.insert(indices.begin(), inds.begin(), inds.end());
                uint32_t m = (inds.back() + 1) - inds.front();
                for (auto& i : inds)
                    i += m;
            }
        }
    }
}