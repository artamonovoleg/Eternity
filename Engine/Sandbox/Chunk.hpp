#pragma once

#include <array>
#include <glm/glm.hpp>

#include "../Eternity.hpp"
#include "Block.hpp"

static const int chunkSize = 6;

class ChunkData
{
    private:
        const int m_Size = chunkSize;
        std::vector<Block> m_Data;
    public:
        ChunkData() : m_Data(chunkSize * chunkSize * chunkSize) {}
        Block& At(const glm::ivec3& pos) 
        { 
            return m_Data[((pos.x * m_Size * m_Size) + (pos.y * m_Size) + pos.z)];
        }
};

class Chunk : public Eternity::Renderable
{
    private:
        glm::vec3               m_Pos;
        const int               m_Size = chunkSize;
        ChunkData               m_ChunkData;
        std::vector<Vertex>&    m_Vertices;
        std::vector<uint32_t>&  m_Indices;

        void PushLeft(const glm::vec3& pos);
        void PushRight(const glm::vec3& pos);
        void PushBottom(const glm::vec3& pos);
        void PushTop(const glm::vec3& pos);
        void PushBack(const glm::vec3& pos);
        void PushFront(const glm::vec3& pos);

        void PushIndices();
        void GenerateMesh();
    public:
        Chunk(glm::ivec3 pos);
};