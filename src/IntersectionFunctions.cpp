#include "IntersectionFunctions.h"

GLuint new_vao_id;
GLuint id_vbo_positions;
GLuint id_vbo_colors;
GLuint fragmentS;
GLuint vertexS;
GLuint GPUprg;

glm::vec4 boxMax;
glm::vec4 boxMin;

glm::mat4 model = Matrix_Identity();

bool jaGerou = false;

void DesenhaCollider(){

    glUseProgram(GPUprg);

    SetCameraToDraw(GPUprg, 1.0f);

    GLint model_matrix_index = glGetUniformLocation(GPUprg, "model");
    glUniformMatrix4fv(model_matrix_index, 1 , GL_FALSE , glm::value_ptr(model));
    //a primeira coisa é passar a matriz de modelagem do objeto pra GPU

    GLint bbox_min_uniform = glGetUniformLocation(GPUprg, "bbox_min");
    GLint bbox_max_uniform = glGetUniformLocation(GPUprg, "bbox_max");
    glUniform4f(bbox_min_uniform, boxMin.x, boxMin.y, boxMin.z, 1.0f);
    glUniform4f(bbox_max_uniform, boxMax.x, boxMax.y, boxMax.z, 1.0f);


    // "Ligamos" o VAO.
    glBindVertexArray(new_vao_id);

    glDrawElements(
        GL_LINE_STRIP,
        10,
        GL_UNSIGNED_INT,
        0
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);

    //"Desligamos" a textura
    //glBindTexture(GL_TEXTURE_2D, 0);
}



bool IntersectionRaySphere(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 sphere_center, float sphere_radius)
{
    glm::vec4 vector_origin_center = ray_origin - sphere_center;

    //printf("intersection ray sphere");

    float b = dotproduct(vector_origin_center, ray_direction);
    float c = dotproduct(vector_origin_center, vector_origin_center) - sphere_radius*sphere_radius;
    float h = b*b - c;
    if(h < 0.0f)
        return false;
    else
        return true;
}


// ARRUMAR
// minimum and maximum are the minimum and maximum extent of the cube
// fonte: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool IntersectionRayCube(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 minimum, glm::vec4 maximum)
{
    float tmin = (minimum.x - ray_origin.x) / ray_direction.x;
    float tmax = (maximum.x - ray_origin.x) / ray_direction.x;

    if (tmin > tmax)
        std::swap(tmin, tmax);

    float tymin = (minimum.y - ray_origin.y) / ray_direction.y;
    float tymax = (maximum.y - ray_origin.y) / ray_direction.y;

    if (tymin > tymax)
        std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (minimum.z - ray_origin.z) / ray_direction.z;
    float tzmax = (maximum.z - ray_origin.z) / ray_direction.z;

    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    return true;
}


glm::vec4 ClosestPointOnOBBToPoint(const OBB obb, const glm::vec4 point) {
     float distance = 0;
     glm::vec4 result = obb.centro;
     glm::vec4 dir = point - obb.centro;

     glm::vec4 eixos[3];
     float tamanhos[3];
     eixos[0] = obb.eixoU; tamanhos[0] = obb.tamanhoU*0.5f;
     eixos[1] = obb.eixoV; tamanhos[1] = obb.tamanhoV*0.5f;
     eixos[2] = obb.eixoW; tamanhos[2] = obb.tamanhoW*0.5f;

     for(int i = 0; i < 3; i++){
         distance = dotproduct(dir, eixos[i]); //norma da projeção de dir sobre o eixo

         if (distance > tamanhos[i]) {
            distance = tamanhos[i];
         }
         else if (distance < -tamanhos[i]) {
            distance = -tamanhos[i];
         }

         result = result + (distance*eixos[i]);
         }

    return result;
 }


std::vector<float> IntersectionOBB_Sphere(OBB thisOBB, Sphere thisSphere){
    std::vector<float> retorno;

    glm::vec4 closestPoint = ClosestPointOnOBBToPoint(thisOBB, thisSphere.centro);
    glm::vec4 pointvec = thisSphere.centro - closestPoint;
    float dist = norm(pointvec);

    //printf("\ndist: %f  raio: %f centro: %f", dist, thisSphere.raio, thisSphere.centro);

    if(dist <= thisSphere.raio){
        retorno.push_back(closestPoint.x);
        retorno.push_back(closestPoint.y);
        retorno.push_back(closestPoint.z);
        retorno.push_back(closestPoint.w);
    }

    return retorno;
}


std::vector<float> IntersectionSphereSphere(Sphere s1, Sphere s2){

    std::vector<float> retorno;

    glm::vec4 distance = s2.centro - s1.centro;
    float sqr_distance = dotproduct(distance, distance);
    float sqr_sum_radius = pow((s1.raio + s2.raio) , 2);

    glm::vec4 distanceNorm = distance/norm(distance);

    glm::vec4 pontoInterseccao = s1.centro + distanceNorm*s1.raio;

    if( sqr_distance >  sqr_sum_radius){
        retorno.push_back(pontoInterseccao.x);
        retorno.push_back(pontoInterseccao.y);
        retorno.push_back(pontoInterseccao.z);
        retorno.push_back(pontoInterseccao.w);
    }

    return retorno;
}











