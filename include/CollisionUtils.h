#ifndef COLLISION_UTILS
#define COLLISION_UTILS
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
#include <sys/stat.h>

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
#include "IntersectionFunctions.h"

#define COEFFICIENT_OF_RESTITUTION 0.8f
#define MAX_DISTANCE_FROM_CAMERA 2.0f
#define COLLISION_WITH_CAMERA_CODE -1

#define CAMERA_OBB_SIZE_U 0.5f
#define CAMERA_OBB_SIZE_V 1.0f
#define CAMERA_OBB_SIZE_W 0.5f


void TestMouseCollision(MouseCollisionType colType, std::vector<SceneObject>& currentScene);
void TestCollisions(std::vector<SceneObject>& currentScene);
OBB DefineOrientedBoundingBox(SceneObject obj);
#endif // COLLISION_UTILS
