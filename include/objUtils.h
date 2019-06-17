#ifndef OBJUTILS
#define OBJUTILS

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
#include <sys/stat.h>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Cria��o de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Cria��o de janelas do sistema operacional

// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cameraUtils.h"
#include "SceneLoadSaveUtils.h"
#include "CollisionUtils.h"
#include "BezierCurvesUtils.h"
#include "LightUtils.h"

void UpdateScreenRatio(float newScreenRatio);
void UpdateFramebufferSize(int newHeight, int newWidth);
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
void SetObjectPosition(glm::vec4 newPosition, SceneObject& obj);
void CallUpdateFuntions();
int GetHeight();
int GetWidth();
void ReloadScene();
void ReloadScene(std::string filename);
int FindObjectIndexByName(std::string name);
void TextRendering_PrintString(const std::string &str, float x, float y, float scale=1.0f);
void SetLightPosition(glm::vec4 newPos);
void SetLightDirection(glm::vec4 newDir);
void SetLightColor(glm::vec4 newColor);
#endif// OBJUTILS
