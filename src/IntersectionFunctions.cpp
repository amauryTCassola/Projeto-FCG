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

    // "Ligamos" a textura vinculada a este objeto, se tiver
    //if(objToDraw.texture_id != -1)
        //glBindTexture(GL_TEXTURE_2D, objToDraw.textureIds[objToDraw.activeTexture]);

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

    /*model = Matrix_Identity();

    glm::vec4 maxA = glm::vec4(thisOBB.tamanhoU/2,    thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxB = glm::vec4(-thisOBB.tamanhoU/2,   thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxC = glm::vec4(-thisOBB.tamanhoU/2,   thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxD = glm::vec4(thisOBB.tamanhoU/2,    thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);

    glm::vec4 minA = glm::vec4(thisOBB.tamanhoU/2,  -thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minB = glm::vec4(-thisOBB.tamanhoU/2, -thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minC = glm::vec4(-thisOBB.tamanhoU/2, -thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minD = glm::vec4(thisOBB.tamanhoU/2,  -thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);


        float arrayPontos[] = {
        maxA.x, maxA.y, maxA.z, 1.0f,
        maxB.x, maxB.y, maxB.z, 1.0f,
        maxC.x, maxC.y, maxC.z, 1.0f,
        maxD.x, maxD.y, maxD.z, 1.0f,
        minA.x, minA.y, minA.z, 1.0f,
        minB.x, minB.y, minB.z, 1.0f,
        minC.x, minC.y, minC.z, 1.0f,
        minD.x, minD.y, minD.z, 1.0f
        };

        float arrayCores[] = {
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f
        };

        GLuint arrayIndices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 4, 0
        };


        glGenVertexArrays(1, &new_vao_id);
        glBindVertexArray(new_vao_id);


        glGenBuffers(1, &id_vbo_positions);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_positions);
            glBufferData(GL_ARRAY_BUFFER, 4*8* sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*8* sizeof(float), arrayPontos);
            GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
            GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



        glGenBuffers(1, &id_vbo_colors);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_colors);
            glBufferData(GL_ARRAY_BUFFER, 4*8* sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*8* sizeof(float), arrayCores);
            location = 1; // "(location = 0)" em "shader_vertex.glsl"
            number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



        GLuint id_vbo_indices;
        glGenBuffers(1, &id_vbo_indices);

        // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(GLuint), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 8 * sizeof(GLuint), arrayIndices);

        fragmentS = CreateFragmentShader("../../src/shader_fragment.glsl");
        vertexS = CreateVertexShader("../../src/shader_vertex.glsl");
        GPUprg = CreateGPUProgram(vertexS, fragmentS);

        glBindVertexArray(0);

        //printf("GEROOOOOUUUUUU ");

        jaGerou = true;
        DesenhaCollider();*/



    return true;
}

/*
// extreme_a e extreme_b são os pontos exatamente no meio dos círculos que definem os limites do cilindro
// fonte: http://www.iquilezles.org/www/articles/intersectors/intersectors.htm
bool IntersectionRayCylinder(glm::vec4 ray_origin, glm::vec4 ray_direction, glm::vec4 extreme_a, glm::vec4 extreme_b, float radius)
{
    glm::vec4 ca = extreme_b - extreme_a;
    glm::vec4 oc = ray_origin - extreme_a;

    float caca = dotproduct(ca, ca);
    float card = dotproduct(ca, ray_direction);
    float caoc = dotproduct(ca, oc);

    float a = caca - card * card;
    float b = caca * dotproduct(oc, ray_direction) - caoc * card;
    float c = caca * dotproduct(oc, oc) - caoc*caoc - radius * radius * caca;
    float h = b * b - a * c;
    if( h < 0.0 )
        return false;

    h = sqrt(h);
    float t = (-b -h) / a;

    float y = caoc + t*card;
    if( y > 0.0 && y < caca )
        return true;

    t = (((y < 0.0)?0.0:caca) - caoc)/card;
    if( abs(b+a*t) < h )
        return true;

    return false;
}


// c1 e c2 são vértices em corners opostos (como bbox.max e bbox.min)
// o cubo tem que estar alinhado com os eixos x,y,z global senão faça mudança de sistemas de coordenadas
// fonte: https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test
bool IntersectionCubeSphere(glm::vec4 c1, glm::vec4 c2, glm::vec4 sphere_center, float radius)
{
    float dist_squared = radius * radius;

    if (sphere_center.x < c1.x)
        dist_squared -= pow((sphere_center.x - c1.x),2);
    else if (sphere_center.x > c2.x)
        dist_squared -= pow((sphere_center.x - c2.x),2);
    if (sphere_center.y < c1.y)
        dist_squared -= pow((sphere_center.y - c1.y),2);
    else if (sphere_center.y > c2.y)
        dist_squared -= pow((sphere_center.y - c2.y),2);
    if (sphere_center.z < c1.z)
        dist_squared -= pow((sphere_center.z - c1.z),2);
    else if (sphere_center.z > c2.z)
        dist_squared -= pow((sphere_center.z - c2.z),2);
    return dist_squared > 0;
}

glm::vec4 ClosestPointInAABB(glm::vec4 AABB_min, glm::vec4 AABB_max, const glm::vec4 point) {
     glm::vec4 result = point;

     result.x = std::min(std::max());


     if(result.x < AABB_Min.x) result.x = AABB_min.x;

     result.x = (result.x<min.x) ? min.x : result.x;
     result.y = (result.y<min.x) ? min.y : result.y;
     result.z = (result.z<min.x) ? min.z : result.z;
     result.x = (result.x>max.x) ? max.x : result.x;
     result.y = (result.y>max.x) ? max.y : result.y;
     result.z = (result.z>max.x) ? max.z : result.z;
     return result;
}
*/

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



   /*         model = Matrix_Identity();

    glm::vec4 maxA = thisOBB.centro+glm::vec4(thisOBB.tamanhoU/2,    thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxB = thisOBB.centro+glm::vec4(-thisOBB.tamanhoU/2,   thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxC = thisOBB.centro+glm::vec4(-thisOBB.tamanhoU/2,   thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 maxD = thisOBB.centro+glm::vec4(thisOBB.tamanhoU/2,    thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);

    glm::vec4 minA = thisOBB.centro+glm::vec4(thisOBB.tamanhoU/2,  -thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minB = thisOBB.centro+glm::vec4(-thisOBB.tamanhoU/2, -thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minC = thisOBB.centro+glm::vec4(-thisOBB.tamanhoU/2, -thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);
    glm::vec4 minD = thisOBB.centro+glm::vec4(thisOBB.tamanhoU/2,  -thisOBB.tamanhoV/2, -thisOBB.tamanhoW/2, 0.0f);


        float arrayPontos[] = {
        maxA.x, maxA.y, maxA.z, 1.0f,
        maxB.x, maxB.y, maxB.z, 1.0f,
        maxC.x, maxC.y, maxC.z, 1.0f,
        maxD.x, maxD.y, maxD.z, 1.0f,
        minA.x, minA.y, minA.z, 1.0f,
        minB.x, minB.y, minB.z, 1.0f,
        minC.x, minC.y, minC.z, 1.0f,
        minD.x, minD.y, minD.z, 1.0f
        };

        float arrayCores[] = {
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f
        };

        GLuint arrayIndices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 4, 0
        };


        glGenVertexArrays(1, &new_vao_id);
        glBindVertexArray(new_vao_id);


        glGenBuffers(1, &id_vbo_positions);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_positions);
            glBufferData(GL_ARRAY_BUFFER, 4*8* sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*8* sizeof(float), arrayPontos);
            GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
            GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



        glGenBuffers(1, &id_vbo_colors);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_colors);
            glBufferData(GL_ARRAY_BUFFER, 4*8* sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*8* sizeof(float), arrayCores);
            location = 1; // "(location = 0)" em "shader_vertex.glsl"
            number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



        GLuint id_vbo_indices;
        glGenBuffers(1, &id_vbo_indices);

        // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(GLuint), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 8 * sizeof(GLuint), arrayIndices);

        fragmentS = CreateFragmentShader("../../src/shader_fragment.glsl");
        vertexS = CreateVertexShader("../../src/shader_vertex.glsl");
        GPUprg = CreateGPUProgram(vertexS, fragmentS);

        glBindVertexArray(0);

        //printf("GEROOOOOUUUUUU ");

        jaGerou = true;
        DesenhaCollider();*/

    return retorno;
}
