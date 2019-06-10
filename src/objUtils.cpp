#include "objUtils.h"

std::vector<SceneObject> currentScene;              //lista de objetos que representa a cena atual
float lastFrameTime = 0;
float screen_ratio = 1.0f;
float deltaTime = 0;
float currentTime = 0;

void DrawVirtualObject(SceneObject objToDraw)
{
    GLuint program_id = objToDraw.gpuProgramId;

    glUseProgram(program_id);

    SetCameraToDraw(program_id, screen_ratio);

    GLint model_matrix_index = glGetUniformLocation(program_id, "model");
    glUniformMatrix4fv(model_matrix_index, 1 , GL_FALSE , glm::value_ptr(objToDraw.model));
    //a primeira coisa é passar a matriz de modelagem do objeto pra GPU

    GLint bbox_min_uniform = glGetUniformLocation(program_id, "bbox_min");
    GLint bbox_max_uniform = glGetUniformLocation(program_id, "bbox_max");
    glUniform4f(bbox_min_uniform, objToDraw.bbox_min_min_min.x, objToDraw.bbox_min_min_min.y, objToDraw.bbox_min_min_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, objToDraw.bbox_max_max_max.x, objToDraw.bbox_max_max_max.y, objToDraw.bbox_max_max_max.z, 1.0f);


    // "Ligamos" o VAO.
    glBindVertexArray(objToDraw.vertex_array_object_id);

    // "Ligamos" a textura vinculada a este objeto, se tiver
    //if(objToDraw.texture_id != -1)
        glBindTexture(GL_TEXTURE_2D, objToDraw.textureIds[objToDraw.activeTexture]);

    glDrawElements(
        objToDraw.rendering_mode,
        objToDraw.num_indices,
        GL_UNSIGNED_INT,
        (void*)objToDraw.first_index
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);

    //"Desligamos" a textura
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawCurrentScene(){
    for(unsigned int i = 0; i<currentScene.size(); i++){
            DrawVirtualObject(currentScene[i]);
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
