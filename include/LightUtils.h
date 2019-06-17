#ifndef LIGHTUTILS
#define LIGHTUTILS

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
#include "objUtils.h"

void DrawShadows(glm::vec4 lightPosition, glm::vec4 lightDirection, std::vector<SceneObject>& currentScene);
GLuint GetShadowMap();
glm::mat4 GetLightMatrix(glm::vec4 lightPosition, glm::vec4 lightDirection);

#endif // LIGHTUTILS
