#include "Scene0Functions.h"


bool sphereActive = false;
bool loadedTex = false;
GLuint texId0, texId1;

void SphereOnClick(std::vector<SceneObject>& _currentScene, int callerIndex){

    if(sphereActive){
        _currentScene[callerIndex].activeTexture = 0;
        _currentScene[callerIndex].velocity = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
        sphereActive = false;
    }
    else{
        sphereActive = true;
        _currentScene[callerIndex].activeTexture = 1;
        _currentScene[callerIndex].velocity = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    }
}

void SphereOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex){
    //printf("A");
}

glm::vec4 lastVelocityNorm(0.0f, 0.0f, 1.0f, 0.0f);

void SphereOnMove(std::vector<SceneObject>& _currentScene, int callerIndex){

    glm::vec4 curVelocityNorm = _currentScene[callerIndex].velocity/norm(_currentScene[callerIndex].velocity);
    float theta = acos(dotproduct(curVelocityNorm, lastVelocityNorm));

    if( theta > PI/4){
            _currentScene[callerIndex].rotationMatrix = Matrix_Identity();
            lastVelocityNorm = curVelocityNorm;
    }


    glm::vec4 axis = crossproduct(_currentScene[callerIndex].velocity, glm::vec4(_currentScene[callerIndex].velocity.x, _currentScene[callerIndex].velocity.y+5, _currentScene[callerIndex].velocity.z, 0.0f));

    axis = axis/norm(axis);

    RotateObject(_currentScene[callerIndex], -axis, norm(_currentScene[callerIndex].velocity*GetDeltaTime()));
}



void SphereChildOnMove(std::vector<SceneObject>& _currentScene, int callerIndex){
    RotateObject(_currentScene[callerIndex], glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), 2*GetDeltaTime());
}


void RabbitOnClick(std::vector<SceneObject>& _currentScene, int callerIndex){
    SceneObject rabbit = _currentScene[callerIndex];
    glm::vec4 rabbitCenter = rabbit.model*((rabbit.bbox_min_min_min+rabbit.bbox_max_max_max)*0.5f);

    if(GetCameraMode() == CameraMode::FREE){
        ActivateLookAtCamera(rabbitCenter, 3.0f);
    }
    else ActivateFreeCamera();
}
