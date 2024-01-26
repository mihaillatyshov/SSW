#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace LM
{

    struct CurcleCurveProps
    {
        float AngleStart;
        float AngleEnd;
        float Radius;
        float CenterX;
        float CenterY;
    };

    void AddCircleCurve(CurcleCurveProps _Props, size_t _Sections, std::vector<glm::vec4>& _Vertices);

    void IndicesAddTriangle(std::vector<uint32_t>& _Indices, uint32_t _VertId0, uint32_t _VertId1, uint32_t _VertId2);

}    // namespace LM
