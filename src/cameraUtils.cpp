#include "cameraUtils.h"

glm::vec4 camera_view_vector        = glm::vec4(0.0f,0.0f,-1.0f,0.0f);;
glm::vec4 camera_up_vector          = glm::vec4(0.0f,1.0f,0.0f,0.0f);
glm::vec4 camera_position_point     = glm::vec4(0.0f,0.0f,2.0f,1.0f);

float camera_speed = 0.01f;
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

bool usePerspectiveProjection = true;

glm::vec4 GetViewVector(){
    return camera_view_vector;
}

glm::vec4 GetUpVector(){
    return camera_up_vector;
}

glm::vec4 GetCameraPosition(){
    return camera_position_point;
}

void UpdateCamera(GLuint program_id, float screen_ratio){

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

void Move(glm::vec4 camera_position_increment_vector){
    camera_position_increment_vector = glm::vec4(camera_position_increment_vector.x, 0.0f, camera_position_increment_vector.z, 0.0f);
    glm::vec4 new_camera_position_point = camera_position_point + camera_position_increment_vector;
    camera_position_point = new_camera_position_point;
}

void MoveCameraForward(){
        glm::vec4 camera_position_increment_vector = (camera_view_vector/norm(camera_view_vector))*camera_speed;
        Move(camera_position_increment_vector);
}

void MoveCameraBack(){
        glm::vec4 camera_position_increment_vector = -(camera_view_vector/norm(camera_view_vector))*camera_speed;
        Move(camera_position_increment_vector);
}

void MoveCameraLeft(){
    glm::vec4 camera_position_increment_vector = (crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)))*camera_speed;
    Move(camera_position_increment_vector);
}

void MoveCameraRight(){
    glm::vec4 camera_position_increment_vector = -(crossproduct(camera_up_vector, camera_view_vector)/norm(crossproduct(camera_up_vector, camera_view_vector)))*camera_speed;
    Move(camera_position_increment_vector);
}

float phi_angle = 0.0f;
void RotateCamera(float dx, float dy){
    glm::vec4 w = -camera_view_vector/norm(camera_view_vector);
    glm::vec4 u = crossproduct(camera_up_vector, w)/norm(crossproduct(camera_up_vector, w));

    camera_view_vector = camera_view_vector*Matrix_Rotate(dx, camera_up_vector);

    float new_phi_angle = phi_angle + dy;
    if(new_phi_angle < -PI/2) dy = (-PI/2) - phi_angle;
    if(new_phi_angle > PI/2) dy = (PI/2) - phi_angle;

    phi_angle += dy;

    camera_view_vector = camera_view_vector*Matrix_Rotate(dy, u);
}

void MoveCamera(bool isPressingW, bool isPressingA, bool isPressingS, bool isPressingD){
    if(isPressingW) MoveCameraForward();
    if(isPressingS) MoveCameraBack();
    if(isPressingA) MoveCameraLeft();
    if(isPressingD) MoveCameraRight();
}
