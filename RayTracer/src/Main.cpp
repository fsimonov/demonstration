#include "Tracer.h"
#include "stdio.h"
#include "glm.hpp"
#include <ctime>
#include <cmath>


void InitializeCamera(glm::uvec2 res, int size = 1024 * 768, glm::vec3 pos = vec3(16e+10, 14e+9, 0), glm::vec3 right = vec3(0.0, 0.0, 10.0), glm::vec3 up = vec3(0.0, 10.0, 0.0), glm::vec3 viewDir = vec3(-10.0, 0.0, 0.0), double viewAngleY = M_PI / 3);
int main(int argc, char** argv)
{
    unsigned int start_time = clock();
    CTracer tracer;
    CScene scene;

    int xRes = 1024;  // Default parameters
    int yRes = 768;
    double M = 8.57e+36;
    double k = 7;
    std::string starsPath = "./bin/data/stars.jpg";
    std::string diskPath = "./bin/data/disk_32.png";
    glm::vec3 pos = glm::vec3(16e+10, 14e+9, 0);
    glm::vec3 forward = glm::vec3(-10.0, 0.0, 0.0);
    glm::vec3 right = glm::vec3(0.0, 0.0, 10.0);
    glm::vec3 up = glm::vec3(0.0, 10.0, 0.0);
    double viewAngleY = M_PI / 3;
    double deltaT = 5;
    int antialiasing = 1;


    if(argc >= 2) // There is input file in parameters
    {
        FILE* file = fopen(argv[1], "r");
        if(file)
        {
            int xResFromFile = 0;
            int yResFromFile = 0;
            double MFromFile = 0;
            double kFromFile = 0;
            double px, py, pz, fx, fy, fz, rx, ry, rz, ux, uy, uz, ay;
            int anti;
            double delta;
            if(fscanf(file, "%d %d %lf %lf", &xResFromFile, &yResFromFile, &MFromFile, &kFromFile) == 4)
            {
                xRes = xResFromFile;
                yRes = yResFromFile;
                M = MFromFile;
                k = kFromFile;
                if(fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &px, &py, &pz, &fx, &fy, &fz, &rx, &ry, &rz, &ux, &uy, &uz, &ay) == 13)
                {
                    pos = vec3(px, py, pz);
                    forward = vec3(fx, fy, fz);
                    right = vec3(rx, ry, rz);
                    up = vec3(ux, uy, uz);
                    viewAngleY = ay;
                    if (fscanf(file, "%lf", &delta) == 1)
                    {
                        deltaT = delta;
                        if (fscanf(file, "%d", &anti) == 1)
                        {
                            antialiasing = anti;
                        }
                        else
                        {
                            printf("There's no antialiasing parameter! Using default parameter.\r\n");
                        }
                    }
                    else
                    {
                        printf("There are no antialiasing and deltaT parameters! Using default parameter.\r\n");
                    }


                }
                else
                {
                    printf("There are no camera, deltaT and antialiasing parameters! Using default parameters.\r\n");
                }
            }
            else
            {
                printf("Invalid config format! Using default parameters.\r\n");
            }

            fclose(file);
        }
        else
        {
            printf("Invalid config path! Using default parameters.\r\n");
        }
    }
    if (argc >= 3) // There is path of image with stars in parameters
    {
        starsPath = argv[2];
    }
    if (argc == 4) // There is path of disk texture in parameters
    {
        diskPath = argv[3];
    }
    if (argc > 4)
    {
        printf("There are too many parameters!\r\n");
    }
    if (argc == 1)
    {
        printf("No config! Using default parameters.\r\n");
    }

    scene.Initialize(M, k);
    tracer.m_pScene = &scene;
    tracer.InitializeCamera(glm::uvec2(xRes, yRes), xRes * yRes, pos, right, up, forward, viewAngleY);
    tracer.RenderImage(xRes, yRes, starsPath, diskPath, antialiasing, deltaT);
    tracer.SaveImageToFile("./img/Result.png");
    unsigned int end_time = clock();
    float time =  (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("TIME: %.0f sec.\n", time);
}
