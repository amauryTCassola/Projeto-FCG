#include "Scene0Functions.h"

#define SPHERE_STATE_INDEX 0
void SphereOnClick(std::vector<bool>& _sceneVariables, std::vector<SceneObject>& _currentScene, int callerIndex){
    if(_sceneVariables[SPHERE_STATE_INDEX]){
        glDeleteTextures(1, &_currentScene[callerIndex].texture_id);
        _currentScene[callerIndex].texture_id = CreateNewTexture("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT);
        _sceneVariables[SPHERE_STATE_INDEX] = false;
    }
    else{
        _sceneVariables[SPHERE_STATE_INDEX] = true;
        glDeleteTextures(1, &_currentScene[callerIndex].texture_id);
        _currentScene[callerIndex].texture_id = CreateNewTexture("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT);
    }
}

void SphereOnMouseOver(std::vector<bool>& _sceneVariables, std::vector<SceneObject>& _currentScene, int callerIndex){
    printf("A");
}
