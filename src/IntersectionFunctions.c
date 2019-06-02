#include "IntersectionFunctions.h"

bool IntersectionRaySphere(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 sphere_center, float sphere_radius){
    glm::vec4 vector_origin_center = ray_origin - sphere_center;

    float b = dotproduct(vector_origin_center, ray_direction);
    float c = dotproduct(vector_origin_center, vector_origin_center) - sphere_radius*sphere_radius;
    float h = b*b - c;
    if(h < 0.0f) return false;
    else return true;


}
