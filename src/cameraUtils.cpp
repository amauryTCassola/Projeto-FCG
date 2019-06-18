#include "cameraUtils.h"

CameraMode currentMode = CameraMode::FREE;
glm::vec4 camera_view_vector        = glm::vec4(0.0f,0.0f,-1.0f,0.0f);
glm::vec4 camera_up_vector          = glm::vec4(0.0f,1.0f,0.0f,0.0f);
glm::vec4 camera_position_point     = glm::vec4(0.0f,0.0f,2.0f,1.0f);
glm::vec4 camera_velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

glm::vec4 camera_point_to_look_at = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem


float camera_movement_speed = 5.0f;
float camera_rotation_speed = 7.0f;

bool wasMovingForward = false, wasMovingBackward = false, wasMovingRight = false, wasMovingLeft = false;
bool isMovingForward = false, isMovingBackward = false, isMovingRight = false, isMovingLeft = false;

float timeMoving = 0.0f;
float totalAccelerationTime = 0.5f;

float rotationX = 0.0f;
float rotationY = 0.0f;

bool usePerspectiveProjection = true;

float lastTimeRotate;

void ActivateLookAtCamera(glm::vec4 pointToLookAt, float distance){
    camera_point_to_look_at = pointToLookAt;
    currentMode = CameraMode::LOOKAT;
    g_CameraDistance = distance;
}

void ActivateFreeCamera(){
    currentMode = CameraMode::FREE;
}

void MoveCameraByVector(glm::vec4 movementVector){
    camera_position_point += movementVector;
}

void SetCameraVelocity(glm::vec4 newVelocity){
    camera_velocity = newVelocity;
}

glm::vec4 GetCameraVelocity(){
    return camera_velocity;
}

glm::vec4 GetCameraViewVector(){
    return camera_view_vector;
}

glm::vec4 GetCameraPosition(){
    return camera_position_point;
}

void SetCameraViewVector(glm::vec4 _view_vector){
    camera_view_vector = _view_vector;
}

void SetCameraPosition(glm::vec4 _camera_position){
    camera_position_point = _camera_position;
}

glm::vec4 GetCameraUpVector(){
    return camera_up_vector;
}

void SetCameraUpVector(glm::vec4 new_up){
    camera_up_vector = new_up;
}

CameraMode GetCameraMode(){
    return currentMode;
}

void SetCameraMode(CameraMode _newMode){
    currentMode = _newMode;
}

glm::vec4 GetLookAtCameraPosition(){
    if(currentMode == CameraMode::FREE)
        return camera_position_point;
    else{
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        return Matrix_Translate(camera_point_to_look_at.x, camera_point_to_look_at.y, camera_point_to_look_at.z)*glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
    }
}

void SetCameraOrtho(){
    usePerspectiveProjection = false;
}

void SetCameraPerspective(){
    usePerspectiveProjection = true;
}

void SetCameraToDraw(GLuint program_id, float screen_ratio){

    GLint view_matrix_index = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    GLint projection_matrix_index = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl

    glm::mat4 view;

    if(currentMode == CameraMode::FREE){

        view = Matrix_Camera_View(camera_position_point, camera_view_vector, camera_up_vector);
    }

    else{
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        glm::vec4 camera_position_c  = Matrix_Translate(camera_point_to_look_at.x, camera_point_to_look_at.y, camera_point_to_look_at.z)*glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_view_vector_lookat = camera_point_to_look_at - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        view = Matrix_Camera_View(camera_position_c, camera_view_vector_lookat, camera_up_vector);
    }

    glm::mat4 projection;
    float nearplane = -0.1f;  // Posição do "near plane"
    float farplane  = -30.0f; // Posição do "far plane"

    if (usePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slide 227 do documento "Aula_09_Projecoes.pdf".
            float field_of_view = PI / 3.0f;
            projection = Matrix_Perspective(field_of_view, screen_ratio, nearplane, farplane);
        }
        else
        {

            float nearplane = -0.1f;  // Posição do "near plane"
            float farplane  = -20.0f; // Posição do "far plane"
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slide 236 do documento "Aula_09_Projecoes.pdf".
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 3.0f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*screen_ratio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glUniformMatrix4fv(view_matrix_index       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_matrix_index , 1 , GL_FALSE , glm::value_ptr(projection));
}

float phi_angle = 0.0f;
float theta_angle = 0.0f;


void RotateCamera(float dx, float dy){

        float currentTime = 0.0f;
        if(lastTimeRotate == 0.0f){
            lastTimeRotate = currentTime - 0.1f;
        }
        float delta = currentTime - lastTimeRotate;
        lastTimeRotate = currentTime;

    if(currentMode == CameraMode::FREE){

        glm::vec4 w = -glm::vec4(0.0f,0.0f,-1.0f,0.0f)/norm(glm::vec4(0.0f,0.0f,-1.0f,0.0f));
        glm::vec4 u = crossproduct(camera_up_vector, w)/norm(crossproduct(camera_up_vector, w));

        float new_phi_angle = (phi_angle + dy*delta*camera_rotation_speed);

        if(new_phi_angle < -PI/3) new_phi_angle = -PI/3;
        if(new_phi_angle > PI/3) new_phi_angle = PI/3;

        phi_angle = new_phi_angle;

        float new_theta_angle = (theta_angle + dx*delta*camera_rotation_speed);
        theta_angle = new_theta_angle;

        camera_view_vector = Matrix_Rotate(-new_theta_angle, camera_up_vector)*Matrix_Rotate(-new_phi_angle, u)*glm::vec4(0.0f,0.0f,-1.0f,0.0f);
    } else {
        g_CameraTheta -= dx;
        g_CameraPhi   += dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = PI/3;
        float phimin = 0;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
    }

}

void UpdateCameraPositionAndRotation(float deltaTime){

    float normalizedValue = 0.0f;

    if((isMovingBackward || isMovingForward || isMovingLeft || isMovingRight)
       && (wasMovingBackward || wasMovingForward || wasMovingLeft || wasMovingRight)){
        timeMoving += deltaTime;
    } else {
        timeMoving = 0.0f;
    }

    normalizedValue = std::min(1.0f, timeMoving/totalAccelerationTime);

    if(currentMode == CameraMode::FREE){
        camera_position_point = camera_position_point + camera_velocity*camera_movement_speed*normalizedValue*deltaTime;
        camera_position_point = glm::vec4(camera_position_point.x, 0.0f, camera_position_point.z, camera_position_point.w);
    }



    RotateCamera(rotationX*deltaTime, rotationY*deltaTime);

    camera_velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    wasMovingForward = isMovingForward;
    wasMovingBackward = isMovingBackward;
    wasMovingLeft = isMovingLeft;
    wasMovingRight = isMovingRight;

    isMovingForward = false;
    isMovingBackward = false;
    isMovingLeft = false;
    isMovingRight = false;

    rotationX = 0;
    rotationY = 0;
}

void MoveCameraForward(float delta){
    isMovingBackward = true;
    camera_velocity +=  (camera_view_vector/norm(camera_view_vector));
    camera_velocity = glm::vec4(camera_velocity.x, 0.0f, camera_velocity.z, 0.0f);
}

void MoveCameraBack(float delta){
    isMovingBackward = true;
    camera_velocity += -(camera_view_vector/norm(camera_view_vector));
    camera_velocity = glm::vec4(camera_velocity.x, 0.0f, camera_velocity.z, 0.0f);
}

void MoveCameraLeft(float delta){
    isMovingLeft = true;
    camera_velocity += (crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)));
    camera_velocity = glm::vec4(camera_velocity.x, 0.0f, camera_velocity.z, 0.0f);
}

void MoveCameraRight(float delta){
    isMovingRight = true;
    camera_velocity += -(crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)));
    camera_velocity = glm::vec4(camera_velocity.x, 0.0f, camera_velocity.z, 0.0f);
}

void AddToCameraRotationX(float addX){
    //printf("\nX");
    rotationX += addX;
}

void AddToCameraRotationY(float addY){
   // printf("\nY");
    rotationY += addY;
}
