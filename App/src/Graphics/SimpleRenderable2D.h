#pragma once

#include <vector>

#include "Engine/Buffers/VertexArray.h"

#include "glm/glm.hpp"

namespace LM
{

    class SimpleRenderable2D
    {
    public:
        Ref<VertexBuffer> m_Vertices;
        Ref<IndexBuffer> m_Indices;
        Ref<VertexArray> m_VertexArray;

        void Create(const std::vector<glm::vec4>& _Vertices, const std::vector<uint32_t>& _Indices);
        void Draw() const;
    };

}    // namespace LM
