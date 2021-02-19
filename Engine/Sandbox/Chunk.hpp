#pragma once

#include <array>
#include <glm/glm.hpp>

#include "../Eternity.hpp"
#include "Block.hpp"

static const int chunkSize = 16;

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
        glm::ivec3              m_Pos;
        const int               m_Size = chunkSize;
        ChunkData               m_ChunkData;

        void GenerateBlocks();
    public:
        Chunk(glm::ivec3 pos);
};