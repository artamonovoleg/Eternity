#pragma once
#include <initializer_list>
#include <vector>
#include <glm/glm.hpp>

struct Block
{
    enum class Type
    {
        Air,
        Ground,
        TopGround
    };

    Type type;
    Block() : type(Type::Air) {}
    Block(Type type) : type(type) {}
};