#include "objUtils.h"

std::vector<SceneObject> currentScene;              //lista de objetos que representa a cena atual
float lastFrameTime = 0;
float screen_ratio = 1.0f;
float deltaTime = 0;
float currentTime = 0;

int framebufferWidth = 0;
int framebufferHeight = 0;

void UpdateFramebufferSize(int newHeight, int newWidth){
    framebufferWidth = newWidth;
    framebufferHeight= newHeight;

}

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

    for(unsigned int i = 0; i<currentScene.size(); i++){
        if(currentScene[i].update != NULL)currentScene[i].update(currentScene, i);
    }

    lastFrameTime = currentTime;
}

float GetDeltaTime(){
    return deltaTime;
}


glm::vec4 GetPointInOBBNormal(OBB thisOBB, glm::vec4 collisionPoint){
    glm::vec4 collisionPointNormal;

    glm::vec4 vectorCenterToPoint = collisionPoint - thisOBB.centro;

    glm::vec4 vectorInOBBSpace = thisOBB.cartesianToOBB*vectorCenterToPoint;

    float diffX = (thisOBB.tamanhoU/2) - std::abs(vectorInOBBSpace.x);
    float diffY = (thisOBB.tamanhoV/2) - std::abs(vectorInOBBSpace.y);
    float diffZ = (thisOBB.tamanhoW/2) - std::abs(vectorInOBBSpace.z);

    glm::vec4 resultVector = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    if(diffX <= diffY && diffX <= diffZ){
        resultVector = glm::vec4(vectorInOBBSpace.x, 0.0f, 0.0f, 0.0f);
    }
    if(diffY <= diffX && diffY <= diffZ){
        resultVector = glm::vec4(resultVector.x, vectorInOBBSpace.y, 0.0f, 0.0f);
    }
    if(diffZ <= diffX && diffZ <= diffY){
        resultVector = glm::vec4(resultVector.x, resultVector.y, vectorInOBBSpace.z, 0.0f);
    }

    collisionPointNormal = thisOBB.OBBToCartesian*resultVector;
    collisionPointNormal = collisionPointNormal/norm(collisionPointNormal);

    return collisionPointNormal;
}

#define NO_INTERSECTION -1
/*Usando Cyrus-Beck Clipping*/
/*retirado do livro Game Physics Cookbook - Gabor Szauer*/
/*Retorna o t correspondente ao ponto de entrada do raio no OBB ou -1 se n�o houver intersec��o*/
float IntersectionPointRay_OBB(glm::vec4 ray_origin, glm::vec4 ray_direction, OBB thisOBB)
{

    glm::vec4 X = thisOBB.eixoU;
    glm::vec4 Y = thisOBB.eixoV;
    glm::vec4 Z = thisOBB.eixoW;

    float obb_size[] = {thisOBB.tamanhoU/2, thisOBB.tamanhoV/2, thisOBB.tamanhoW/2};

    glm::vec4 p = thisOBB.centro - ray_origin;
    glm::vec4 f = glm::vec4( dotproduct(X, ray_direction), dotproduct(Y, ray_direction), dotproduct(Z, ray_direction), 0.0f);
    glm::vec4 e = glm::vec4(dotproduct(X, p), dotproduct(Y, p), dotproduct(Z, p), 0.0f);

    float t[6] = { 0, 0, 0, 0, 0, 0 };
    for (int i = 0; i < 3; ++i)
    {
        if( (fabsf((f[i])-(0)) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(f[i]), fabsf(0))) ))
        {
            if (-e[i] - obb_size[i]>0 || -e[i] + obb_size[i]<0)
            {
                return false;
            }
            f[i] = 0.00001f; // Avoid div by 0!
        }
        t[i * 2 + 0] = (e[i] + obb_size[i]) / f[i]; // min
        t[i * 2 + 1] = (e[i] - obb_size[i]) / f[i]; // max
    }

    float tmin = fmaxf(fmaxf(
                           fminf(t[0], t[1]),
                           fminf(t[2], t[3])),
                       fminf(t[4], t[5])
                      );
    float tmax = fminf(
                     fminf(
                         fmaxf(t[0], t[1]),
                         fmaxf(t[2], t[3])),
                     fmaxf(t[4], t[5])
                 );

    if(tmax < 0)
        return NO_INTERSECTION;
    if(tmin > tmax)
        return NO_INTERSECTION;

    return tmin;
}

//Dados um objeto em forma de caixa (que ser� o espelho), uma cor (para mapear a reflex�o, tipo green screen) e um n�mero que representa qual das faces da caixa � a reflexiva,
//implementa um espelho simples da seguinte forma (esta fun��o deve ser chamada ao fim do frame, logo antes de realizar o SwapBuffers:
//1. Salva todas as informa��es relevantes da c�mera (ela ser� movida durante esta fun��o e, depois, deve ser posta de volta no seu lugar)
//2. Salva o framebuffer atual em mem�ria
//3. Tra�a um raio da posi��o da c�mera at� o centro da face reflexiva do espelho
//4. Descobre se este raio intersecta a caixa em algum outro ponto
//  4.1. Se sim, pula para o passo 13
//  4.2. Se n�o, continua
//5. Usando o raio como vetor de incid�ncia, calcula o vetor de reflex�o especular ideal
//6. Move a c�mera para o ponto do espelho que foi calculado no passo 4 e define seu modo como FREE
//7. Define o vetor view da c�mera como o vetor de reflex�o especular ideal calculado no passo 5
//8. Desenha a cena inteira
//9. Salva o framebuffer
//10. Itera sobre o framebuffer original
//  10.1. Se a cor do pixel atual for a mirrorColor, troca ele pelo pixel correspondente no framebuffer salvo no passo 9
//11. Envia o framebuffer para ser desenhado na tela
//12. Libera o espa�o do segundo framebuffer
//13. Restaura as configura��es da c�mera e libera o espa�o alocado para o buffer
//14. Termina

void DrawMirror(SceneObject& mirrorObj, glm::vec4 mirrorColor, MirrorReflectiveFace reflectiveFace){
    /*Passo 1*/
    glm::vec4 originalFreeCameraPosition = GetCameraPosition();
    glm::vec4 originalCameraViewVector = GetCameraViewVector();
    CameraMode originalCameraMode = GetCameraMode();


    /*Passo 2*/
    GLubyte* originalFramebuffer = (GLubyte*)malloc(3*framebufferHeight*framebufferWidth*sizeof(GLubyte));
    //glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);
    glReadPixels(0,0,framebufferWidth, framebufferHeight, GL_RGB, GL_UNSIGNED_BYTE, originalFramebuffer);


    /*Passo 3*/
    OBB mirrorOBB = DefineOrientedBoundingBox(mirrorObj);
    glm::vec4 camera_position;
    switch(originalCameraMode){
    case CameraMode::FREE:
        camera_position = originalFreeCameraPosition;
    break;
    case CameraMode::LOOKAT:
        camera_position = GetLookAtCameraPosition();
    break;
    }

    glm::vec4 reflective_face_center_point = mirrorOBB.centro;
    switch(reflectiveFace){
        case MirrorReflectiveFace::RIGHT:
            reflective_face_center_point += mirrorOBB.eixoU*(mirrorOBB.tamanhoU/2);
        break;
        case MirrorReflectiveFace::LEFT:
            reflective_face_center_point += mirrorOBB.eixoU*(-mirrorOBB.tamanhoU/2);
        break;
        case MirrorReflectiveFace::TOP:
            reflective_face_center_point += mirrorOBB.eixoV*(mirrorOBB.tamanhoV/2);
        break;
        case MirrorReflectiveFace::BOTTOM:
            reflective_face_center_point += mirrorOBB.eixoV*(-mirrorOBB.tamanhoV/2);
        break;
        case MirrorReflectiveFace::FRONT:
            reflective_face_center_point += mirrorOBB.eixoW*(mirrorOBB.tamanhoW/2);
        break;
        case MirrorReflectiveFace::BACK:
            reflective_face_center_point += mirrorOBB.eixoW*(-mirrorOBB.tamanhoW/2);
        break;
    }

    glm::vec4 incidenceVector = reflective_face_center_point - camera_position;
    incidenceVector = incidenceVector/norm(incidenceVector);

    /*Passo 4*/
    float t = IntersectionPointRay_OBB(camera_position, incidenceVector, mirrorOBB);
    glm::vec4 intersection_point = camera_position + incidenceVector*t;



    glm::vec4 face_center_point_normal = GetPointInOBBNormal(mirrorOBB, reflective_face_center_point);
    glm::vec4 intersection_point_normal = GetPointInOBBNormal(mirrorOBB, intersection_point);

    if(face_center_point_normal == intersection_point_normal){

        /*Passo 5*/
        glm::vec4 reflection_vector = incidenceVector - 2*dot(incidenceVector, intersection_point_normal)*intersection_point_normal;

        reflection_vector = glm::vec4(reflection_vector.x, reflection_vector.y, reflection_vector.z, 0.0f);

        /*Passo 6*/
        SetCameraPosition(intersection_point+intersection_point_normal);
        SetCameraMode(CameraMode::FREE);

        /*Passo 7*/
        SetCameraViewVector(reflection_vector);

        glClear(GL_COLOR_BUFFER_BIT);
        /*Passo 8*/
        DrawCurrentScene();


        /*Passo 9*/
        GLubyte* newFramebuffer = (GLubyte*)malloc(3*framebufferHeight*framebufferWidth*sizeof(GLubyte));
        glReadPixels(0,0,framebufferWidth, framebufferHeight, GL_RGB, GL_UNSIGNED_BYTE, newFramebuffer);

        /*Passo 10*/
        for(int i = 0; i<3*framebufferHeight*framebufferWidth; i += 3){

            /*Passo 10.1*/
            if(originalFramebuffer[i+0] == mirrorColor.x
               && originalFramebuffer[i+1] == mirrorColor.y
               && originalFramebuffer[i+2] == mirrorColor.z){
                    originalFramebuffer[i+0] = newFramebuffer[i+0];
                    originalFramebuffer[i+1] =  newFramebuffer[i+1];
                    originalFramebuffer[i+2] =  newFramebuffer[i+2];

               }
        }
        /*Passo 11*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/
        /*??????????????????????????????????????????????????????????????????*/


        //glClear(GL_COLOR_BUFFER_BIT);
        //void glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data);
        /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glRasterPos2i(100, 100);
        glEnable(GL_DEPTH_TEST);*/

        //glDrawPixels(framebufferWidth, framebufferHeight, GL_RGB, GL_UNSIGNED_BYTE, newFramebuffer);

        /*Passo 12*/
        free(newFramebuffer);
    }


    /*Passo 13*/
    free(originalFramebuffer);
    SetCameraPosition(originalFreeCameraPosition);
    SetCameraViewVector(originalCameraViewVector);
    SetCameraMode(originalCameraMode);

}
