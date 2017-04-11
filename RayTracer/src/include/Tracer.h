#pragma once

#include "glm.hpp"
#include "Types.h"
#include "Scene.h"
#include "FreeImagePlus.h"

#include "string"

using namespace glm;

class CTracer
{
public:
    SRay MakeRay(glm::uvec2 pixelPos, glm::vec2 shift);  // Create ray for specified pixel
    glm::vec3 TraceRay(SRay ray, fipImage* pImageStars, fipImage* pImageDisk, double delta); // Trace ray, compute its color
    void RenderImage(int xRes, int yRes, std::string starsPath, std::string diskPath, int antialiasing, double deltaT);
    void SaveImageToFile(std::string fileName);
    fipImage* LoadImageFromFile(std::string fileName);
    void InitializeCamera(glm::uvec2 res, int size, glm::vec3 pos, glm::vec3 right, glm::vec3 up, glm::vec3 viewDir, double viewAngleY);

public:
    SCamera m_camera;
    CScene* m_pScene;
};
