#include "cameraUtils.h"

glm::vec4 camera_view_vector        = glm::vec4(0.0f,0.0f,-1.0f,0.0f);
glm::vec4 camera_up_vector          = glm::vec4(0.0f,1.0f,0.0f,0.0f);
glm::vec4 camera_position_point     = glm::vec4(0.0f,0.0f,2.0f,1.0f);
glm::vec4 camera_velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

float camera_movement_speed = 5.0f;
float camera_rotation_speed = 7.0f;

float g_CameraDistance = 3.5f; // Distância da câmera para a origem

bool wasMovingForward = false, wasMovingBackward = false, wasMovingRight = false, wasMovingLeft = false;
bool isMovingForward = false, isMovingBackward = false, isMovingRight = false, isMovingLeft = false;

float timeMoving = 0.0f;
float totalAccelerationTime = 0.5f;

float rotationX = 0.0f;
float rotationY = 0.0f;

bool usePerspectiveProjection = true;

float lastTimeRotate;

void MoveCameraByVector(glm::vec4 movementVector){
    camera_position_point += movementVector;
}

void SetCameraVelocity(glm::vec4 newVelocity){
    camera_velocity = newVelocity;
}

glm::vec4 GetCameraVelocity(){
    return camera_velocity;
}

glm::vec4 GetViewVector(){
    return camera_view_vector;
}

glm::vec4 GetUpVector(){
    return camera_up_vector;
}

glm::vec4 GetCameraPosition(){
    return camera_position_point;
}

void SetCameraToDraw(GLuint program_id, float screen_ratio){

    GLint view_matrix_index = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    GLint projection_matrix_index = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl

    glm::mat4 view = Matrix_Camera_View(camera_position_point, camera_view_vector, camera_up_vector);
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
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slide 236 do documento "Aula_09_Projecoes.pdf".
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_CameraDistance/2.5f;
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

    glm::vec4 w = -glm::vec4(0.0f,0.0f,-1.0f,0.0f)/norm(glm::vec4(0.0f,0.0f,-1.0f,0.0f));
    glm::vec4 u = crossproduct(camera_up_vector, w)/norm(crossproduct(camera_up_vector, w));

    float new_phi_angle = (phi_angle + dy*delta*camera_rotation_speed);

    if(new_phi_angle < -PI/3) new_phi_angle = -PI/3;
    if(new_phi_angle > PI/3) new_phi_angle = PI/3;

    phi_angle = new_phi_angle;

    float new_theta_angle = (theta_angle + dx*delta*camera_rotation_speed);
    theta_angle = new_theta_angle;

    camera_view_vector = Matrix_Rotate(-new_theta_angle, camera_up_vector)*Matrix_Rotate(-new_phi_angle, u)*glm::vec4(0.0f,0.0f,-1.0f,0.0f);

    //camera_view_vector = Matrix_Rotate(-new_theta_angle, camera_up_vector)*glm::vec4(0.0f,0.0f,-1.0f,0.0f);
    //printf("\n%f", theta_angle);
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

    if(isMovingForward){
        camera_velocity +=  (camera_view_vector/norm(camera_view_vector))*normalizedValue;
    }
    if(isMovingBackward){
        camera_velocity += -(camera_view_vector/norm(camera_view_vector))*normalizedValue;
    }
    if(isMovingLeft){
        camera_velocity += (crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)))*normalizedValue;
    }
    if(isMovingRight){
        camera_velocity += -(crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)))*normalizedValue;
    }


    camera_position_point = camera_position_point + glm::vec4(camera_velocity.x, 0.0f, camera_velocity.z, 0.0f)*deltaTime*camera_movement_speed;
    camera_velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    wasMovingForward = isMovingForward;
    wasMovingBackward = isMovingBackward;
    wasMovingLeft = isMovingLeft;
    wasMovingRight = isMovingRight;

    isMovingForward = false;
    isMovingBackward = false;
    isMovingLeft = false;
    isMovingRight = false;

    RotateCamera(rotationX*deltaTime, rotationY*deltaTime);
    rotationX = 0;
    rotationY = 0;
}

void MoveCameraForward(){
    isMovingForward = true;
}

void MoveCameraBack(){
    isMovingBackward = true;
}

void MoveCameraLeft(){
    isMovingLeft = true;
}

void MoveCameraRight(){
    isMovingRight = true;
}

void AddToCameraRotationX(float addX){
    //printf("\nX");
    rotationX += addX;
}

void AddToCameraRotationY(float addY){
   // printf("\nY");
    rotationY += addY;
}
