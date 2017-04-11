#pragma once

#include "Types.h"
#include "glm.hpp"

const double S_G = 6.674e-11; //G
const double S_c = 3e+8; //light speed

class CScene
{
public:
    glm::vec3 s_centerHole;
    double s_rHole;
    double s_M;
    glm::vec3 s_centerDisk;
    glm::vec3 s_normDisk;
    double s_rDisk;
public:
    void Initialize(double M, double k, glm::vec3 center = glm::vec3(0, 0, 0), glm::vec3 normDisk = glm::vec3(0, 10, 0));
};
