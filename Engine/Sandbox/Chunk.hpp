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

        const Block& At(const glm::ivec3& pos) const
        {
            return m_Data.at(((pos.x * m_Size * m_Size) + (pos.y * m_Size) + pos.z));
        }

        const Block::Type LeftType(const glm::ivec3& pos) const
        {
            if (pos.x == 0)
                return Block::Type::Air;
            else
                return At({ pos.x - 1, pos.y, pos.z }).type;
        };

        const Block::Type RightType(const glm::ivec3& pos) const
        {
            if (pos.x == m_Size - 1)
                return Block::Type::Air;
            else
                return At({ pos.x + 1, pos.y, pos.z }).type;
        }

        const Block::Type BottomType(const glm::ivec3& pos) const
        {
            if (pos.y == 0)
                return Block::Type::Air;
            else
                return At({ pos.x, pos.y - 1, pos.z }).type;
        }

        const Block::Type TopType(const glm::ivec3& pos) const
        {
            if (pos.y == m_Size - 1)
                return Block::Type::Air;
            else
                return At({ pos.x, pos.y + 1, pos.z }).type;
        }

        const Block::Type BackType(const glm::ivec3& pos) const
        {
            if (pos.z == 0)
                return Block::Type::Air;
            else
                return At({ pos.x, pos.y, pos.z - 1 }).type;
        }

        const Block::Type FrontType(const glm::ivec3& pos) const
        {
            if (pos.z == m_Size - 1)
                return Block::Type::Air;
            else
                return At({ pos.x, pos.y, pos.z + 1 }).type;
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

        // Push face into given pos. (In chunk coordinate system not world global)
        void PushLeft(const glm::vec3& pos);
        void PushRight(const glm::vec3& pos);
        void PushBottom(const glm::vec3& pos);
        void PushTop(const glm::vec3& pos);
        void PushBack(const glm::vec3& pos);
        void PushFront(const glm::vec3& pos);

        /// Add 6 more indices
        void PushIndices();

        /// Generate landscape and save map setup to chunk data
        void GenerateLandscape();
        /// Push all vertices and indices
        void GenerateMesh();
    public:
        Chunk(glm::ivec3 pos);
};