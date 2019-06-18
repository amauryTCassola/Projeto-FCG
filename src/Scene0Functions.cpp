#include "Scene0Functions.h"
#include "TextRenderingUtils.h"
#include "SFXUtils.h"

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
    DrawText("Uma bola", TextPosition::CENTER);
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
        ActivateLookAtCamera(rabbitCenter, 2.0f);
    }
    else ActivateFreeCamera();
}

bool isGavetaOpen = false;
bool isOpening = false;
bool isClosing =false;
void GavetaOnClick(std::vector<SceneObject>& _currentScene, int callerIndex){

    if(!isOpening && !isClosing){
         if(!isGavetaOpen){
            isOpening = true;
            isClosing = false;
            PlaySound("../../sfx/Desk Drawer Open 01.wav", false, 1.0f);
        } else{
            isOpening = false;
            isClosing = true;
            PlaySound("../../sfx/Desk Drawer Close 01.wav", false, 1.0f);
        }
    }
}

void GavetaUpdate(std::vector<SceneObject>& _currentScene, int callerIndex){

    if(isOpening){

            int chave1Index = FindObjectIndexByName("chave1");

        if(_currentScene[callerIndex].translationMatrix[3][2] < 200.0f){
            _currentScene[callerIndex].translationMatrix = Matrix_Translate(0.0f, 0.0f, 600*GetDeltaTime())*_currentScene[callerIndex].translationMatrix;
            _currentScene[chave1Index].translationMatrix = Matrix_Translate(-10*GetDeltaTime(), 0.0f, 0.0f)*_currentScene[chave1Index].translationMatrix;
        }
        else{
                isOpening = false;
                isGavetaOpen = true;
        }
    } else if(isClosing){
        int chave1Index = FindObjectIndexByName("chave1");
        if(_currentScene[callerIndex].translationMatrix[3][2] > 0){
            _currentScene[callerIndex].translationMatrix = Matrix_Translate(0.0f, 0.0f, -600*GetDeltaTime())*_currentScene[callerIndex].translationMatrix;
            _currentScene[chave1Index].translationMatrix = Matrix_Translate(10*GetDeltaTime(), 0.0f, 0.0f)*_currentScene[chave1Index].translationMatrix;
        }
        else{
                isClosing = false;
                isGavetaOpen = false;
        }
    }
}

void GavetaOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawText("Uma gaveta", TextPosition::CENTER);
}

glm::vec4 p1 = glm::vec4(2.0f, 4.0, 0.0f, 1.0f);
glm::vec4 p2 = glm::vec4(5.0f, 4.0, 0.0f, 1.0f);
glm::vec4 p3 = glm::vec4(5.0f, -4.0, 0.0f, 1.0f);
glm::vec4 p4 = glm::vec4(2.0f, -4.0, 0.0f, 1.0f);

glm::vec4 p5 = glm::vec4(2.0f, -4.0, 0.0f, 1.0f);
glm::vec4 p6 = glm::vec4(-5.0f, -4.0, 0.0f, 1.0f);
glm::vec4 p7 = glm::vec4(-5.0f, 4.0, 0.0f, 1.0f);
glm::vec4 p8 = glm::vec4(2.0f, 4.0, 0.0f, 1.0f);

std::vector<glm::vec4> pontos {p1, p2, p3, p4, p5, p6, p7, p8};

void SphereChildUpdate(std::vector<SceneObject>& _currentScene, int callerIndex){
    float t = fmod(glfwGetTime(), 2);
    glm::vec4 ponto = PointInBezierCurve(pontos, t);
    SetObjectPosition(ponto, _currentScene[callerIndex]);
}



glm::vec4 magenta = glm::vec4(255.0f, 0.0f, 255.0f, 1.0f);

void MirrorUpdate(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawMirror(_currentScene[callerIndex], magenta, MirrorReflectiveFace::BACK);
}

void DescricaoDummy(std::vector<SceneObject>& _currentScene, int callerIndex){

    DrawText("'Culpa'", TextPosition::CENTER);

}

void DescricaoVenus(std::vector<SceneObject>& _currentScene, int callerIndex){

    DrawText("'Aesthetic'", TextPosition::CENTER);

}

void DescricaoGourard(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawText("'Gouraud'", TextPosition::CENTER);
}

void DescricaoBunny(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawText("'Placeholder'", TextPosition::CENTER);
}

void ChaveOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawText("Uma chave", TextPosition::CENTER);
}

bool isExaminando = false;

void VidroDummyOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex){

    if(!isExaminando)
    DrawText("Clique para examinar", TextPosition::CENTER);

}

void VidroDummyOnClick(std::vector<SceneObject>& _currentScene, int callerIndex){
    SceneObject vidro = _currentScene[callerIndex];
    glm::vec4 vidroCenter = vidro.model*((vidro.bbox_min_min_min+vidro.bbox_max_max_max)*0.5f);

    if(GetCameraMode() == CameraMode::FREE){
        isExaminando = true;
        ActivateLookAtCamera(vidroCenter, 2.0f);
    }
    else{
        ActivateFreeCamera();
        isExaminando = false;
    }
}

void AbstrataDummyOnMouseOver(std::vector<SceneObject>& _currentScene, int callerIndex){

    DrawText("Alguma pintura abstrata", TextPosition::CENTER);

}

void DescricaoChest(std::vector<SceneObject>& _currentScene, int callerIndex){

    DrawText("Um bau", TextPosition::CENTER);

}

void DescricaoBau(std::vector<SceneObject>& _currentScene, int callerIndex){

    DrawText("'Absolutamente vazio'", TextPosition::CENTER);

}

void DescricaoStarryNight(std::vector<SceneObject>& _currentScene, int callerIndex){
    DrawText("'Starry Night' - Van Gogh", TextPosition::CENTER);
}

bool isBauAberto = false;
bool isAnimandoBau = false;
float currentAngleZ = 0;

void AbreBau(std::vector<SceneObject>& _currentScene, int callerIndex){

    if(!isAnimandoBau){
        isAnimandoBau = true;
        if(isBauAberto){
            currentAngleZ = -3*PI/4;
            PlaySound("../../sfx/chest_close.mp3", false, 1.0f);
        } else {
            currentAngleZ = 0;
            PlaySound("../../sfx/chest_open.mp3", false, 1.0f);
        }
    }
}

void AnimacaoBau(std::vector<SceneObject>& _currentScene, int callerIndex){


    if(isAnimandoBau){
            int tampaBau = _currentScene[callerIndex].childrenIndices[0];
        if(!isBauAberto){
            if(currentAngleZ > -3*PI/4){
                currentAngleZ -= 5*GetDeltaTime();
                _currentScene[tampaBau].rotationMatrix *= Matrix_Rotate_Z(-5*GetDeltaTime());
            }
            else{
                isAnimandoBau = false;
                isBauAberto = true;
                _currentScene[tampaBau].rotationMatrix = Matrix_Rotate_Z(-3*PI/4);
            }
        } else {
            if(currentAngleZ < 0){
                currentAngleZ += 5*GetDeltaTime();
                _currentScene[tampaBau].rotationMatrix *= Matrix_Rotate_Z(5*GetDeltaTime());
            }
            else{
                isAnimandoBau = false;
                isBauAberto = false;
                _currentScene[tampaBau].rotationMatrix = Matrix_Rotate_Z(0);
            }
        }
    }
}


float timePassed = 0;
float waitTime = 0;
float lightningTime = 1;
bool isLightning = false;
bool isWaiting = false;
LightMode originalLightMode;
int thunderSound;

void LightningGeneratorUpdate(std::vector<SceneObject>& _currentScene, int callerIndex){
    if(!isLightning){
        if(!isWaiting){
            waitTime = (rand() % 90) + 15;
            isWaiting = true;
        }
        else{
            timePassed += GetDeltaTime();
            if(timePassed >= waitTime){
                isWaiting= false;
                isLightning= true;
                timePassed = 0;
                originalLightMode = GetLightMode();
                SetLightMode(LightMode::LIGHTNING);
                thunderSound = (rand() % 2) + 1;

                if(thunderSound == 1)
                    PlaySound("../../sfx/thunder1.mp3", false, 1.0f);
                else if(thunderSound == 2)
                    PlaySound("../../sfx/thunder2.mp3", false, 1.0f);
            }
        }
    } else{
        timePassed += GetDeltaTime();
        if(timePassed >= lightningTime){
            isLightning = false;
            timePassed = 0;
            SetLightMode(originalLightMode);
        }
    }

}
