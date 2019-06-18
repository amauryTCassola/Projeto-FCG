#include "objUtils.h"

std::vector<SceneObject> currentScene;              //lista de objetos que representa a cena atual
float lastFrameTime = 0;
float screen_ratio = 1.0f;
float deltaTime = 0;
float currentTime = 0;

int framebufferWidth = 0;
int framebufferHeight = 0;

std::string currentSceneName;

void UpdateFramebufferSize(int newHeight, int newWidth){
    framebufferWidth = newWidth;
    framebufferHeight= newHeight;

}

int GetHeight(){
    return framebufferHeight;
}

int GetWidth(){
    return framebufferWidth;
}


glm::vec4 lightPosition;
glm::vec4 lightDirection;
glm::vec4 lightColor;
bool isFlashlight = false;;

void DrawVirtualObject(SceneObject objToDraw)
{
    GLuint program_id = objToDraw.gpuProgramId;

    glUseProgram(program_id);

    SetCameraToDraw(program_id, screen_ratio);

    GLint model_matrix_index = glGetUniformLocation(program_id, "model");
    glUniformMatrix4fv(model_matrix_index, 1 , GL_FALSE , glm::value_ptr(objToDraw.model));
    //a primeira coisa � passar a matriz de modelagem do objeto pra GPU

    GLint bbox_min_uniform = glGetUniformLocation(program_id, "bbox_min");
    GLint bbox_max_uniform = glGetUniformLocation(program_id, "bbox_max");
    glUniform4f(bbox_min_uniform, objToDraw.bbox_min_min_min.x, objToDraw.bbox_min_min_min.y, objToDraw.bbox_min_min_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, objToDraw.bbox_max_max_max.x, objToDraw.bbox_max_max_max.y, objToDraw.bbox_max_max_max.z, 1.0f);

    glUniform4f(glGetUniformLocation(program_id, "lightPos"), lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);
    glUniform4f(glGetUniformLocation(program_id, "lightDir"), lightDirection.x, lightDirection.y, lightDirection.z, lightDirection.w);
    glUniform4f(glGetUniformLocation(program_id, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform1i(glGetUniformLocation(program_id, "isFlashlight"), isFlashlight);


    // "Ligamos" o VAO.
    glBindVertexArray(objToDraw.vertex_array_object_id);

    GLint texLoc = glGetUniformLocation(program_id, "tex");
    glUniform1i(texLoc, 0);

    texLoc = glGetUniformLocation(program_id, "secondaryTex");
    glUniform1i(texLoc, 1);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, objToDraw.textureIds[objToDraw.activeTexture]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, objToDraw.textureIds[objToDraw.activeTexture+1]);


    glDrawElements(
        objToDraw.rendering_mode,
        objToDraw.num_indices,
        GL_UNSIGNED_INT,
        (void*)objToDraw.first_index
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);

    //"Desligamos" a textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CallUpdateFuntions(){
    for(unsigned int i = 0; i<currentScene.size(); i++){
        if(currentScene[i].update != NULL)currentScene[i].update(currentScene, i);
    }
}

void SetLightPosition(glm::vec4 newPos){
    lightPosition = newPos;
}
void SetLightDirection(glm::vec4 newDir){
    lightDirection = newDir;
}
void SetLightColor(glm::vec4 newColor){
    lightColor = newColor;
}

LightMode currentLightMode;

void SetLightMode(LightMode mode){

    currentLightMode = mode;

    if(mode == LightMode::FLASHLIGHT){
        isFlashlight = true;
        SetLightColor(glm::vec4(0.293f, 0.0f, 1.0f, 1.0f));
    } else if(mode == LightMode::DARK){
        isFlashlight = false;
        SetLightPosition(glm::vec4(0.0f, 10.0f, 0.0f, 1.0f));
        SetLightDirection(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        SetLightColor(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    } else if(mode == LightMode::LIGHTNING){
        isFlashlight = false;
        SetLightPosition(glm::vec4(0.0f, 10.0f, 0.0f, 1.0f));
        SetLightDirection(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        SetLightColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    } else if(mode == LightMode::NOLIGHT){
        isFlashlight = false;
        SetLightPosition(glm::vec4(0.0f, 10.0f, 0.0f, 1.0f));
        SetLightDirection(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        SetLightColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

LightMode GetLightMode(){
    return currentLightMode;
}

void DrawCurrentScene(){
    if(isFlashlight){
         SetLightPosition(GetCameraPosition());
        SetLightDirection(GetCameraViewVector());
    }

    for(unsigned int i = 0; i<currentScene.size(); i++){
            if(currentScene[i].active)DrawVirtualObject(currentScene[i]);
       }
}

void ScaleObject(glm::vec4 scaleVector, SceneObject& obj){
    obj.scaleMatrix = Matrix_Scale(scaleVector.x, scaleVector.y, scaleVector.z) * obj.scaleMatrix;
}

void ResetScale(SceneObject& obj){
    obj.scaleMatrix = Matrix_Identity();
}

void MoveObject(glm::vec4 movementVector, SceneObject& obj){
    obj.translationMatrix = Matrix_Translate(movementVector.x, movementVector.y, movementVector.z) * obj.translationMatrix;
}

void SetObjectPosition(glm::vec4 newPosition, SceneObject& obj){
    obj.translationMatrix = Matrix_Translate(newPosition.x, newPosition.y, newPosition.z);
}

void ResetTranslation(SceneObject& obj){
    obj.translationMatrix = Matrix_Identity();
}

void RotateObject(SceneObject& obj, glm::vec4 axis, float angle){
    obj.rotationMatrix = Matrix_Rotate(angle, axis)*obj.rotationMatrix;
}
void ResetRotation(SceneObject& obj){
    obj.rotationMatrix = Matrix_Identity();
}

void ApplyTransformations(SceneObject& obj){
    obj.model = obj.parentMatrix*obj.scaleMatrix * obj.translationMatrix * obj.rotationMatrix;
    if(obj.childrenIndices.size() > 0){
        for(unsigned int i = 0; i<obj.childrenIndices.size(); i++){
            currentScene[obj.childrenIndices[i]].parentMatrix = obj.model;
        }
    }
}

void SaveCurrentScene(std::string filename){
    SaveScene(filename, currentScene);
}

void LoadToCurrentScene(std::string filename){
    OpenScene(filename, currentScene);
}

void LoadToCurrentSceneAdditive(std::string filename){
    OpenSceneAdditive(filename, currentScene);
}

void UnloadCurrentScene(){
    UnloadScene(currentScene);
}

void ReloadScene(std::string filename){
    UnloadCurrentScene();
    LoadToCurrentScene(filename);
}

void UpdateScreenRatio(float newScreenRatio){
    screen_ratio = newScreenRatio;
}

void TestOnClick(){
    TestMouseCollision(MouseCollisionType::CLICK, currentScene);
}

void TestOnMouseOver(){
    TestMouseCollision(MouseCollisionType::MOUSE_OVER, currentScene);
}

void TestPhysicalCollisions(){
    TestCollisions(currentScene);
}

void ApplyFriction(SceneObject& obj, float deltaTime){

    obj.velocity = obj.velocity*(1 - obj.decelerationRate*deltaTime);

    if(norm(obj.velocity) <= 0.1f) obj.velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void CallChildrenMoveFunction(SceneObject parent){
    for(unsigned int i = 0; i<parent.childrenIndices.size(); i++){
        if(currentScene[parent.childrenIndices[i]].onMove != NULL)currentScene[parent.childrenIndices[i]].onMove(currentScene, parent.childrenIndices[i]);
        if(currentScene[parent.childrenIndices[i]].childrenIndices.size() > 0)
            CallChildrenMoveFunction(currentScene[parent.childrenIndices[i]]);
    }
}

void MoveCurrentSceneObjects(){
    glm::vec4 movementVector;
    currentTime = (float)glfwGetTime();

    deltaTime = currentTime - lastFrameTime;

    for(unsigned int i = 0; i<currentScene.size(); i++){
            ApplyFriction(currentScene[i], deltaTime);
            if(currentScene[i].velocity != glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)){
                movementVector = currentScene[i].velocity*currentScene[i].blockMovement*deltaTime;
                MoveObject(movementVector, currentScene[i]);
                if(currentScene[i].onMove != NULL)currentScene[i].onMove(currentScene, i);
                    CallChildrenMoveFunction(currentScene[i]);
            }

            ApplyTransformations(currentScene[i]);
    }
}



void RotateCameraX(float x){
    AddToCameraRotationX(x);
}
void RotateCameraY(float y){
    AddToCameraRotationY(y);
}

void MoveCamera(bool W, bool A, bool S, bool D){

    if(W) MoveCameraForward(deltaTime);
    if(A) MoveCameraLeft(deltaTime);
    if(S) MoveCameraBack(deltaTime);
    if(D) MoveCameraRight(deltaTime);
}


void Debug_CreateNewObjectSphere(){
    Debug_NewObjectSphere(currentScene);
}

void FinishFrame(){
    UpdateCameraPositionAndRotation(deltaTime);
    lastFrameTime = currentTime;
}

float GetDeltaTime(){
    return deltaTime;
}

int FindObjectIndexByName(std::string name){

    for(unsigned int i = 0; i<currentScene.size(); i++){
        if(currentScene[i].name.compare(name) == 0)
            return i;
    }

    return -1;
}
