#ifndef OBJUTILS
#define OBJUTILS

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

#include "cameraUtils.h"
#include "SceneLoadSaveUtils.h"
#include "CollisionUtils.h"

void UpdateScreenRatio(float newScreenRatio);

void DrawCurrentScene();

void SaveCurrentScene(std::string filename);
void LoadToCurrentScene(std::string filename);
void LoadToCurrentSceneAdditive(std::string filename);

void Debug_CreateNewObjectSphere();

void TestPhysicalCollisions();
void TestOnMouseOver();
void TestOnClick();

void RotateCameraX(float x);
void RotateCameraY(float y);

void MoveCamera(bool W, bool A, bool S, bool D);

void MoveCurrentSceneObjects();

void ChangeTexture(int index, std::string filename);

void ScaleObject(glm::vec4 scaleVector, SceneObject& obj);
void ResetScale(SceneObject& obj);
void MoveObject(glm::vec4 movementVector, SceneObject& obj);
void ResetTranslation(SceneObject& obj);
void RotateObject(SceneObject& obj, glm::vec4 axis, float angle);
void ResetRotation(SceneObject& obj);
float GetDeltaTime();
void FinishFrame();
#endif// OBJUTILS
