#include "Tracer.h"
#include <iostream>
#include <cmath>

using namespace glm;

SRay CTracer::MakeRay(glm::uvec2 pixelPos, glm::vec2 shift)
{
    SRay ray;
    ray.m_start = m_camera.m_pos;
    double w = m_camera.m_resolution.x;
    double h = m_camera.m_resolution.y;
    double cx = pixelPos.x;
    double cy = pixelPos.y;
    ray.m_dir = m_camera.m_forward;
    glm::vec3 right = m_camera.m_right;
    right *= ((cx + shift.x) / w - 0.5);
    ray.m_dir += right;
    glm::vec3 up = m_camera.m_up;
    up *= ((cy + shift.y) / h - 0.5);
    ray.m_dir += up;
    return ray;
}

glm::vec3 CTracer::TraceRay(SRay ray, fipImage* pImageStars, fipImage* pImageDisk, double delta)
{
    auto pDataStars = (unsigned char*)pImageStars->accessPixels();
    int pitch = pImageStars->getScanWidth();
    glm::vec3 curPos = ray.m_start;
    curPos += ray.m_dir;
    glm::vec3 curDir = glm::normalize(ray.m_dir);
    curDir *= S_c; //3e+8 light speed
    double deltaT = delta;
    double alpha = 0.0;
    glm::vec3 color(0, 0, 0);
    bool ind = false;
    glm::vec3 v = curPos;
    v -= m_pScene->s_rHole;
    double dist = glm::length(v);
    if (dist < m_pScene->s_rDisk)
    {
        dist *= 2.0;
        dist += m_pScene->s_rDisk;
    }
    else
    {
        dist *= 2.5;
    }
    dist /= S_c;
    dist /= deltaT;
    int cicles =  dist;
    for (int i = 0; i < cicles; i++)
    {
        double minT = deltaT;
        double R = m_pScene->s_rHole;
        double a = curDir.x * curDir.x + curDir.y * curDir.y + curDir.z * curDir.z;
        double b = 2.0 * (curPos.x * curDir.x + curPos.y * curDir.y + curPos.z * curDir.z);
        double c = curPos.x * curPos.x + curPos.y * curPos.y + curPos.z * curPos.z - R * R;
        double D = b * b - 4 * a * c;
        if (D > 0)
        {
            minT = min((-b - sqrt(D)) / (2.0 * a), (-b + sqrt(D)) / (2.0 * a));
            if (minT > 0.0 && minT < deltaT)
            {
                glm::vec3 color1 = vec3(0, 0, 0);
                color1 *= (1.0 - alpha);
                color += color1;
                ind = true;
            }
        }
        double t = m_pScene->s_centerDisk.x * m_pScene->s_normDisk.x;
        t += m_pScene->s_centerDisk.y * m_pScene->s_normDisk.y;
        t += m_pScene->s_centerDisk.z * m_pScene->s_normDisk.z;
        t -= curPos.x * m_pScene->s_normDisk.x;
        t -= curPos.y * m_pScene->s_normDisk.y;
        t -= curPos.z * m_pScene->s_normDisk.z;
        double scal = curDir.x * m_pScene->s_normDisk.x;
        scal += curDir.y * m_pScene->s_normDisk.y;
        scal += curDir.z * m_pScene->s_normDisk.z;
        if (abs(scal) > 0)
        {
            t /= scal;
            if (t > 0.0 && t < deltaT && t < minT)
            {
                glm::vec3 pointDisk = curPos;
                glm::vec3 shift = curDir;
                shift *= t;
                pointDisk += shift;
                pointDisk -= m_pScene->s_centerDisk;
                double len = glm::length(pointDisk);
                if (len < m_pScene->s_rDisk)
                {
                    // Reading input texture sample

                    if(pImageDisk->getBitsPerPixel() == 32)
                    {
                        int h = pImageDisk->getHeight();
                        int w = pImageDisk->getWidth();
                        vec2 texture = vec2(pointDisk.x, pointDisk.z);
                        texture /= (m_pScene->s_rDisk * 2.0);
                        texture += 0.5;
                        texture.x *= h;
                        texture.y *= w;
                        int i = texture.x;
                        int j = texture.y;
                        auto pDataDisk = (unsigned char*)pImageDisk->accessPixels();
                        int pitch = pImageDisk->getScanWidth();
                        double b = pDataDisk[pitch * i + j * 4];
                        double g = pDataDisk[pitch * i + j * 4 + 1];
                        double r = pDataDisk[pitch * i + j * 4 + 2];
                        alpha = pDataDisk[pitch * i + j * 4 + 3];

                        alpha /= 255.0;
                        b /= 255.0;
                        g /= 255.0;
                        r /= 255.0;
                        glm::vec3 color1 = vec3(r, g, b);
                        color1 *= alpha;
                        color += color1;
                    }
                    else
                    {
                        int h = pImageDisk->getHeight();
                        int w = pImageDisk->getWidth();
                        vec2 texture = vec2(pointDisk.x, pointDisk.z);
                        texture /= (m_pScene->s_rDisk * 2.0);
                        texture += 0.5;
                        texture.x *= h;
                        texture.y *= w;
                        int i = texture.x;
                        int j = texture.y;
                        auto pDataDisk = (unsigned char*)pImageDisk->accessPixels();
                        int pitch = pImageDisk->getScanWidth();
                        double b = pDataDisk[pitch * i + j * 3];
                        double g = pDataDisk[pitch * i + j * 3 + 1];
                        double r = pDataDisk[pitch * i + j * 3 + 2];
                        b /= 255.0;
                        g /= 255.0;
                        r /= 255.0;
                        if (!(r < 0.15 && g < 0.15 && b < 0.15))
                        {
                            alpha = 1.0;
                            glm::vec3 color1 = vec3(r, g, b);
                            color1 *= alpha;
                            color += color1;
                            ind = true;
                        }
                    }
                }
            }
        }
        if (ind)
        {
            break;
        }

        glm::vec3 aHole = m_pScene->s_centerHole;
        aHole -= curPos;
        double lenR = glm::length(aHole);
        aHole *= (S_G * m_pScene->s_M / (lenR * lenR * lenR));
        double cosA = (aHole.x * curDir.x + aHole.y * curDir.y + aHole.z * curDir.z) / (glm::length(aHole) * glm::length(curDir));
        glm::vec3 aNorm = curDir;
        aNorm *= (cosA * glm::length(aHole) / glm::length(curDir));
        aNorm *= -1.0;
        aNorm += aHole;
        glm::vec3 a1 = aNorm;
        a1 *= (deltaT * deltaT / 2.0);
        glm::vec3 path = curDir;
        path *= deltaT;
        curPos += path;
        curPos += a1;
        glm::vec3 aV = aNorm;
        aV *= deltaT;
        curDir += aV;

    }
    if (!ind)
    {
        glm::vec3 normDir = glm::normalize(curDir);
        double phi = atan2(-normDir.z, normDir.x);
        double theta = asin(normDir.y);
        phi /= (2.0 * M_PI);
        phi *= pImageStars->getWidth();
        theta += (M_PI / 2.0);
        theta /= M_PI;
        theta *= pImageStars->getHeight();
        int width = phi;
        int height = (theta - 150);
        height %= pImageStars->getHeight();
        double b = pDataStars[pitch * height + 3 * width];
        double g = pDataStars[pitch * height + 3 * width + 1];
        double r = pDataStars[pitch * height + 3 * width + 2];
        b /= 255.0;
        g /= 255.0;
        r /= 255.0;
        glm::vec3 color1 = vec3(r, g, b);
        color1 *= (1.0 - alpha);
        color += color1;
    }
    return color;
}

void CTracer::InitializeCamera(glm::uvec2 res, int size, glm::vec3 pos, glm::vec3 right, glm::vec3 up, glm::vec3 viewDir, double viewAngleY)
{
    m_camera.m_pos = pos;
    m_camera.m_forward = glm::normalize(viewDir);
    m_camera.m_right = glm::normalize(right);
    m_camera.m_up = glm::normalize(up);
    m_camera.m_viewAngle.y = viewAngleY;
    m_camera.m_resolution = res;
    m_camera.m_pixels.resize(size);
    double len = res.y / 2;
    len /= tan(viewAngleY / 2.0);
    m_camera.m_viewAngle.x = atan2(res.x / 2, len) * 2.0;
    m_camera.m_forward *= len;
    m_camera.m_right *= res.x;
    m_camera.m_up *= res.y;

}

void CTracer::RenderImage(int xRes, int yRes, std::string starsPath, std::string diskPath, int antialiasing, double deltaT)
{


    // Rendering
    fipImage* pImageStars = LoadImageFromFile(starsPath);
    fipImage* pImageDisk = LoadImageFromFile(diskPath);
    for(int i = 0; i < yRes; i++)
    {
        for(int j = 0; j < xRes; j++)
        {
            if (antialiasing == 1)
            {
                SRay ray = MakeRay(uvec2(j, i), vec2(0.5, 0.5));
                m_camera.m_pixels[i * xRes + j] = TraceRay(ray, pImageStars, pImageDisk, deltaT);
            }
            else if (antialiasing == 2)
            {
                SRay ray = MakeRay(uvec2(j, i), vec2(0.5, 0.25));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.5, 0.75));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                m_camera.m_pixels[i * xRes + j] /= 2.0;
            }
            else if (antialiasing == 4)
            {
                SRay ray = MakeRay(uvec2(j, i), vec2(0.25, 0.25));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.25, 0.75));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.75, 0.25));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.75, 0.75));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                m_camera.m_pixels[i * xRes + j] /= 4.0;
            }
            else if (antialiasing == 16)
            {
                SRay ray = MakeRay(uvec2(j, i), vec2(0.2, 0.2));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.2, 0.4));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.2, 0.6));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.2, 0.8));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);

                ray = MakeRay(uvec2(j, i), vec2(0.4, 0.2));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.4, 0.4));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.4, 0.6));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.4, 0.8));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);

                ray = MakeRay(uvec2(j, i), vec2(0.6, 0.2));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.6, 0.4));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.6, 0.6));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.6, 0.8));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);

                ray = MakeRay(uvec2(j, i), vec2(0.8, 0.2));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.8, 0.4));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.8, 0.6));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);
                ray = MakeRay(uvec2(j, i), vec2(0.8, 0.8));
                m_camera.m_pixels[i * xRes + j] += TraceRay(ray, pImageStars, pImageDisk, deltaT);

                m_camera.m_pixels[i * xRes + j] /= 16.0;
            }
            else
            {
                printf("There are only 1,2,4 parameters available for antialiasing.\r\n");
            }


        }
    }
}

void CTracer::SaveImageToFile(std::string fileName)
{
    fipImage image;

    int width = m_camera.m_resolution.x;
    int height = m_camera.m_resolution.y;

    image.setSize(FIT_BITMAP, width, height, 24);

    int pitch = image.getScanWidth();
    unsigned char* imageBuffer = (unsigned char*)image.accessPixels();

    if (pitch < 0)
    {
        imageBuffer += pitch * (height - 1);
        pitch = -pitch;
    }

    int i, j;
    int imageDisplacement = 0;
    int textureDisplacement = 0;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            vec3 color = m_camera.m_pixels[textureDisplacement + j];

            imageBuffer[imageDisplacement + j * 3] = clamp(color.b, 0.0f, 1.0f) * 255.0f;
            imageBuffer[imageDisplacement + j * 3 + 1] = clamp(color.g, 0.0f, 1.0f) * 255.0f;
            imageBuffer[imageDisplacement + j * 3 + 2] = clamp(color.r, 0.0f, 1.0f) * 255.0f;
        }

        imageDisplacement += pitch;
        textureDisplacement += width;
    }

    image.save(fileName.c_str());
    image.clear();
}

fipImage* CTracer::LoadImageFromFile(std::string fileName)
{
    fipImage *pImage = new fipImage();

    if(pImage->load(fileName.c_str()))
        return pImage;
    else
    {
        delete pImage;
        return NULL;
    }
}
