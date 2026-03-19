#pragma once 

#include <vector>
#include <string>
#include <random>

namespace Fox {

    float randomFloat(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

}
