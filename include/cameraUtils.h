#ifndef CAMERA_UTILS
#define CAMERA_UTILS
#define PI 3.14159265359f

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

#include "matrices.h"


void UpdateCamera(GLuint program_id, float screen_ratio);
void MoveCameraForward();
void MoveCameraBack();
void MoveCameraLeft();
void MoveCameraRight();
void RotateCamera(float dx, float dy);
glm::vec4 GetCameraPosition();
glm::vec4 GetUpVector();
glm::vec4 GetViewVector();

#endif // CAMERA_UTILS
