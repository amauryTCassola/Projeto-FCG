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

enum class CameraMode{FREE, LOOKAT};

void SetCameraToDraw(GLuint program_id, float screen_ratio);
void UpdateCameraPositionAndRotation(float deltaTime);
glm::vec4 GetCameraPosition();
glm::vec4 GetUpVector();
glm::vec4 GetViewVector();
glm::vec4 GetCameraVelocity();
void AddToCameraRotationX(float addX);
void AddToCameraRotationY(float addY);
void MoveCameraForward(float delta);
void MoveCameraBack(float delta);
void MoveCameraLeft(float delta);
void MoveCameraRight(float delta);
void MoveCameraByVector(glm::vec4 movementVector);
void SetCameraVelocity(glm::vec4 newVelocity);
CameraMode GetCameraMode();
void ActivateFreeCamera();
void ActivateLookAtCamera(glm::vec4 pointToLookAt, float distance);

#endif // CAMERA_UTILS
