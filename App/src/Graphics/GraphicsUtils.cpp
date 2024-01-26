#include "GraphicsUtils.h"

#include "Calculations/Steps.h"

namespace LM
{

    void AddCircleCurve(CurcleCurveProps _Props, size_t _Sections, std::vector<glm::vec4>& _Vertices)
    {
        for (size_t i = 0; i <= _Sections; i++)
        {
            float angle = ValueByStep(_Props.AngleStart, _Props.AngleEnd, i, _Sections);

            float x = _Props.Radius * glm::cos(glm::radians(angle)) + _Props.CenterX;
            float y = _Props.Radius * glm::sin(glm::radians(angle)) + _Props.CenterY;

            _Vertices.emplace_back(x, y, 0.0f, 1.0f);
        }
    }

    void IndicesAddTriangle(std::vector<uint32_t>& _Indices, uint32_t _VertId0, uint32_t _VertId1, uint32_t _VertId2)
    {
        _Indices.emplace_back(_VertId0);
        _Indices.emplace_back(_VertId1);
        _Indices.emplace_back(_VertId2);
    }

}    // namespace LM
