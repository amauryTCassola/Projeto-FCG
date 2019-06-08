
#ifndef INTERSECTIONFUNCTIONS
#define INTERSECTIONFUNCTIONS

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
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
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
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

typedef struct Interval {
    float Min;
    float Max;
} Interval;


//IMPLEMENTAR FUNÇÕES COMENTADAS, algoritmos podem ser encontrados em:
//http://www.realtimerendering.com/intersections.html
bool IntersectionRaySphere(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 sphere_center, float sphere_radius);
bool IntersectionRayCube(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 minimum, glm::vec4 maximum);
bool IntersectionRayOBB(glm::vec4 ray_origin, glm::vec4 ray_direction, OBB thisOBB);


std::vector<float> IntersectionOBB_Sphere(OBB thisOBB, Sphere thisSphere);
std::vector<float> IntersectionSphereSphere(Sphere s1, Sphere s2);


Interval GetInterval(OBB thisOBB, glm::vec4 axis);
bool OverlapOnAxis(OBB obb1, OBB obb2, glm::vec4 axis);
bool IntersectionOBB_OBB(OBB obb1, OBB obb2);




#endif // INTERSECTIONFUNCTIONS
