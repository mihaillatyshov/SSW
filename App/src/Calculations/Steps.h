#pragma once

#include <vector>

namespace LM
{

    float ValueByStep(float _Min, float _Max, int _Step, int _StepsCount);

    std::vector<float> GenValueByStep(float _Min, float _Max, int _StepsCount);

    std::vector<int> GenSteps(int _StepsCount);

}    // namespace LM
