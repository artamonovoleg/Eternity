#include "Chunk.hpp"
#include "Block.hpp"

Chunk::Chunk(glm::ivec3 pos)
    : m_Vertices(vertices), m_Indices(indices), m_Pos(pos)
{
    GenerateLandscape();
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
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5)    + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5)   + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5)     + m_Pos + pos, .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    m_Vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5)    + m_Pos + pos, .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });

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

void Chunk::GenerateLandscape()
{
    for (int z = 0; z < m_Size; z++)
    {
        for (int y = 0; y < m_Size; y++)
        {
            for (int x = 0; x < m_Size; x++)
            {
                m_ChunkData.At({x, y, z}).type = Block::Type::TopGround;
            }
        }
    }
}

void Chunk::GenerateMesh()
{
    for (int z = 0; z < m_Size; z++)
    {
        for (int y = 0; y < m_Size; y++)
        {
            for (int x = 0; x < m_Size; x++)
            {
                if (m_ChunkData.At({x, y, z}).type != Block::Type::Air)
                {
                    if (m_ChunkData.LeftType({ x, y, z }) == Block::Type::Air)
                        PushLeft({ x, y, z });
                    if (m_ChunkData.RightType({ x, y, z }) == Block::Type::Air)
                        PushRight({ x, y, z });
                    if (m_ChunkData.BackType({ x, y, z }) == Block::Type::Air)
                        PushBack({ x, y, z });
                    if (m_ChunkData.FrontType({ x, y, z }) == Block::Type::Air)
                        PushFront({x, y, z });
                    if (m_ChunkData.BottomType({ x, y, z }) == Block::Type::Air)
                        PushBottom({ x, y, z });
                    if (m_ChunkData.TopType({ x, y, z }) == Block::Type::Air)
                        PushTop({ x, y, z });
                }
            }
        }
    }
}