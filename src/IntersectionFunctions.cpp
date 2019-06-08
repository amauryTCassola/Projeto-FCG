#include "IntersectionFunctions.h"
#include <cfloat>

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

void DesenhaCollider()
{

    glUseProgram(GPUprg);

    SetCameraToDraw(GPUprg, 1.0f);

    GLint model_matrix_index = glGetUniformLocation(GPUprg, "model");
    glUniformMatrix4fv(model_matrix_index, 1, GL_FALSE, glm::value_ptr(model));
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

bool IntersectionRayOBB(glm::vec4 ray_origin, glm::vec4 ray_direction, OBB thisOBB)
{
    glm::vec4 X = thisOBB.eixoU;
    glm::vec4 Y = thisOBB.eixoV;
    glm::vec4 Z = thisOBB.eixoW;

    float obb_size[] = {thisOBB.tamanhoU, thisOBB.tamanhoV, thisOBB.tamanhoW};

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
        return false;
    if(tmin > tmax)
        return false;

    return true;
}


glm::vec4 ClosestPointOnOBBToPoint(const OBB obb, const glm::vec4 point)
{
    float distance = 0;
    glm::vec4 result = obb.centro;
    glm::vec4 dir = point - obb.centro;

    glm::vec4 eixos[3];
    float tamanhos[3];
    eixos[0] = obb.eixoU;
    tamanhos[0] = obb.tamanhoU*0.5f;
    eixos[1] = obb.eixoV;
    tamanhos[1] = obb.tamanhoV*0.5f;
    eixos[2] = obb.eixoW;
    tamanhos[2] = obb.tamanhoW*0.5f;

    for(int i = 0; i < 3; i++)
    {
        distance = dotproduct(dir, eixos[i]); //norma da projeção de dir sobre o eixo

        if (distance > tamanhos[i])
        {
            distance = tamanhos[i];
        }
        else if (distance < -tamanhos[i])
        {
            distance = -tamanhos[i];
        }

        result = result + (distance*eixos[i]);
    }

    return result;
}


std::vector<float> IntersectionOBB_Sphere(OBB thisOBB, Sphere thisSphere)
{
    std::vector<float> retorno;

    glm::vec4 closestPoint = ClosestPointOnOBBToPoint(thisOBB, thisSphere.centro);
    glm::vec4 pointvec = thisSphere.centro - closestPoint;
    float dist = norm(pointvec);

    //printf("\ndist: %f  raio: %f centro: %f", dist, thisSphere.raio, thisSphere.centro);

    if(dist <= thisSphere.raio)
    {
        retorno.push_back(closestPoint.x);
        retorno.push_back(closestPoint.y);
        retorno.push_back(closestPoint.z);
        retorno.push_back(closestPoint.w);
    }

    return retorno;
}


std::vector<float> IntersectionSphereSphere(Sphere s1, Sphere s2)
{

    std::vector<float> retorno;

    glm::vec4 distance = s2.centro - s1.centro;
    float sqr_distance = dotproduct(distance, distance);
    float sqr_sum_radius = pow((s1.raio + s2.raio), 2);

    glm::vec4 distanceNorm = distance/norm(distance);

    glm::vec4 pontoInterseccao = s1.centro + distanceNorm*s1.raio;

    if( sqr_distance >  sqr_sum_radius)
    {
        retorno.push_back(pontoInterseccao.x);
        retorno.push_back(pontoInterseccao.y);
        retorno.push_back(pontoInterseccao.z);
        retorno.push_back(pontoInterseccao.w);
    }


    return retorno;
}


Interval GetInterval(OBB thisOBB, glm::vec4 axis)
{
    glm::vec4 vertex[8];
    glm::vec4 C = thisOBB.centro;
    glm::vec4 E = glm::vec4(thisOBB.tamanhoU, thisOBB.tamanhoV, thisOBB.tamanhoW, 0.0f);
    glm::vec4 A[] = { thisOBB.eixoU, thisOBB.eixoV, thisOBB.eixoW };

    vertex[0] = C + A[0]*E[0] + A[1]*E[1] + A[2]*E[2];
    vertex[1] = C - A[0]*E[0] + A[1]*E[1] + A[2]*E[2];
    vertex[2] = C + A[0]*E[0] - A[1]*E[1] + A[2]*E[2];
    vertex[3] = C + A[0]*E[0] + A[1]*E[1] - A[2]*E[2];
    vertex[4] = C - A[0]*E[0] - A[1]*E[1] - A[2]*E[2];
    vertex[5] = C + A[0]*E[0] - A[1]*E[1] - A[2]*E[2];
    vertex[6] = C - A[0]*E[0] + A[1]*E[1] - A[2]*E[2];
    vertex[7] = C - A[0]*E[0] - A[1]*E[1] + A[2]*E[2];

    Interval result;
    result.Min = result.Max = dotproduct(axis, vertex[0]);
    for (int i = 1; i < 8; ++i)
    {
        float projection = dotproduct(axis, vertex[i]);
        result.Min = (projection < result.Min) ? projection : result.Min;
        result.Max = (projection > result.Max) ? projection : result.Max;
    }

    return result;
}


bool OverlapOnAxis(OBB obb1, OBB obb2, glm::vec4 axis)
{
    Interval a = GetInterval(obb1, axis);
    Interval b = GetInterval(obb2, axis);
    return ((b.Min <= a.Max) && (a.Min <= b.Max));
}


std::vector<float> IntersectionOBB_OBB(OBB obb1, OBB obb2)
{

    std::vector<float> retorno;

    glm::vec4 test[15] =
    {
        obb1.eixoU,
        obb1.eixoV,
        obb1.eixoW,
        obb2.eixoU,
        obb2.eixoV,
        obb2.eixoW
    };

    for (int i = 0; i < 3; ++i)   // Fill out rest of axis
    {
        test[6 + i * 3 + 0] = crossproduct(test[i], test[0]);
        test[6 + i * 3 + 1] = crossproduct(test[i], test[1]);
        test[6 + i * 3 + 2] = crossproduct(test[i], test[2]);
    }

    for (int i = 0; i < 15; ++i)
    {
        if (!OverlapOnAxis(obb1, obb2, test[i]))
        {
            return retorno;
        }
    }

    glm::vec4 pontoInterseccao = ClosestPointOnOBBToPoint(obb1, obb2.centro);

    retorno.push_back(pontoInterseccao.x);
    retorno.push_back(pontoInterseccao.y);
    retorno.push_back(pontoInterseccao.z);
    retorno.push_back(pontoInterseccao.w);

    return retorno;
}


