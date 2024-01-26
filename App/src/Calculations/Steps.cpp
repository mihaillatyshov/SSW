#include "Steps.h"

namespace LM
{

    float ValueByStep(float _Min, float _Max, int _Step, int _StepsCount)
    {
        return _Min + (_Max - _Min) * float(_Step) / float(_StepsCount);
    }

    std::vector<float> GenValueByStep(float _Min, float _Max, int _StepsCount)
    {
        std::vector<float> result;
        for (int i = 0; i <= _StepsCount; i++)
        {
            result.emplace_back(ValueByStep(_Min, _Max, i, _StepsCount));
        }
        return result;
    }

    std::vector<int> GenSteps(int _StepsCount)
    {
        std::vector<int> result;
        for (int i = 0; i <= _StepsCount; i++)
        {
            result.emplace_back(i);
        }
        return result;
    }

}    // namespace LM
