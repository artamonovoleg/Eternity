#include "Chunk.hpp"
#include "Block.hpp"
///
static  std::vector<Vertex> block { 
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

static std::vector<uint32_t> inds { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };
///

Chunk::Chunk(glm::ivec3 pos)
    : m_Vertices(vertices), m_Indices(indices), m_Pos(pos)
{
	GenerateMesh();
}

void Chunk::PushLeft(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5)   + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5)    + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });

    PushIndices();
}

void Chunk::PushRight(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5)   + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5)     + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });

    PushIndices();
}

void Chunk::PushBottom(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(2.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5)   + m_Pos + pos, .texCoord = glm::vec2(2.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });

    PushIndices();
}

void Chunk::PushTop(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(0, 0) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(0, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5)     + m_Pos + pos, .texCoord = glm::vec2(1.0f / 16.0f, 0) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5)      + m_Pos + pos, .texCoord = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) });

    PushIndices();
}

void Chunk::PushBack(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5)   + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5)     + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });

    PushIndices();
}

void Chunk::PushFront(const glm::vec3& pos)
{
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5)      + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });

    PushIndices();
}

void Chunk::PushIndices()
{
    uint32_t m;
    if (!m_Indices.empty())
        m = m_Indices.back() + 1;
    else
        m = 0;
    m_Indices.insert(m_Indices.end(), { 0 + m, 1 + m, 2 + m, 2 + m, 1 + m, 3 + m });
}

void Chunk::GenerateMesh()
{
    
    for (int z = 0; z < m_Size; z++)
    {
        for (int y = 0; y < m_Size; y++)
        {
            for (int x = 0; x < m_Size; x++)
            {
                PushLeft({x, y, z});
                PushRight({x, y, z});
                PushBack({x, y, z});
                PushFront({x, y, z});
                PushBottom({x, y, z});
                PushTop({x, y, z});
            }
        }
    }
}