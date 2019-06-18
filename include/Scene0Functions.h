#ifndef SCENE0FUNTIONS
#define SCENE0FUNTIONS

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
#include "objUtils.h"
#include "cameraUtils.h"
#include "BezierCurvesUtils.h"
#include "MirrorUtils.h"
#define PI 3.14159265359f

void SphereOnClick(std::vector<SceneObject>& currentScene, int callerIndex);
void SphereOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex);
void SphereOnMove(std::vector<SceneObject>& _currentScene, int callerIndex);
void SphereChildOnMove(std::vector<SceneObject>& _currentScene, int callerIndex);
void RabbitOnClick(std::vector<SceneObject>& _currentScene, int callerIndex);
void SphereChildUpdate(std::vector<SceneObject>& _currentScene, int callerIndex);
void MirrorUpdate(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoDummy(std::vector<SceneObject>& _currentScene, int callerIndex);
void GavetaOnClick(std::vector<SceneObject>& _currentScene, int callerIndex);
void GavetaUpdate(std::vector<SceneObject>& _currentScene, int callerIndex);
void GavetaOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex);
void VidroDummyOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex);
void VidroDummyOnClick(std::vector<SceneObject>& _currentScene, int callerIndex);
void AbstrataDummyOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoVenus(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoGourard(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoBunny(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoChest(std::vector<SceneObject>& _currentScene, int callerIndex);
void LightningGeneratorUpdate(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoBau(std::vector<SceneObject>& _currentScene, int callerIndex);
void DescricaoStarryNight(std::vector<SceneObject>& _currentScene, int callerIndex);
void AbreBau(std::vector<SceneObject>& _currentScene, int callerIndex);
void AnimacaoBau(std::vector<SceneObject>& _currentScene, int callerIndex);
void ChaveOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex);
#endif // SCENE0FUNTIONS
