#ifndef INTERSECTIONFUNCTIONS
#define INTERSECTIONFUNCTIONS

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo s�o espec�ficos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Cria��o de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Cria��o de janelas do sistema operacional

// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneLoadSaveUtils.h"
#include "matrices.h"
#include "cameraUtils.h"

struct OBB{

    glm::vec4 eixoU;
    glm::vec4 eixoV;
    glm::vec4 eixoW;

    glm::vec4 centro;

    float tamanhoU;
    float tamanhoV;
    float tamanhoW;

    glm::mat4 cartesianToOBB;
    glm::mat4 OBBToCartesian;

};


struct Sphere{

    glm::vec4 centro;
    float raio;

};

struct Cylinder{

    glm::vec4 eixo;
    glm::vec4 centro;
    float raio;

};

//IMPLEMENTAR FUN��ES COMENTADAS, algoritmos podem ser encontrados em:
//http://www.realtimerendering.com/intersections.html
bool IntersectionRaySphere(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 sphere_center, float sphere_radius);
bool IntersectionRayCube(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 minimum, glm::vec4 maximum);
//bool IntersectionRayCylinder();


//bool IntersectionOBB_OBB();
//bool IntersectionOBB_Cylinder();
std::vector<float> IntersectionOBB_Sphere(OBB thisOBB, Sphere thisSphere);
//bool IntersectionCylinder_Cylinder();
//bool IntersectionCylinder_Sphere();
//bool IntersectionSphere_Sphere();

#endif // INTERSECTIONFUNCTIONS
