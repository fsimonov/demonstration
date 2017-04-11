#include "Scene.h"


void CScene::Initialize(double M, double k, glm::vec3 center, glm::vec3 normDisk)
{
    s_centerHole = center;
    s_centerDisk = center;
    s_M = M;
    s_rHole = 2 * S_G * s_M / (S_c * S_c);
    s_rDisk = k * s_rHole;
    s_normDisk = normDisk;
}
