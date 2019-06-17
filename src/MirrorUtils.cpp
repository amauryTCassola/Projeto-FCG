#include "MirrorUtils.h"


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
/*Retorna o t correspondente ao ponto de entrada do raio no OBB ou -1 se não houver intersecção*/
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

void UnbindFrameBuffer(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, GetWidth(), GetHeight());

}

void BindFrameBuffer(GLuint frameBuffer, int width, int height){

    glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
}

GLuint CreateDepthAttachment(GLuint fbo, int width, int height){
    GLuint depth_buffer_id;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenRenderbuffers(1, &depth_buffer_id);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_id);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return depth_buffer_id;
}

GLuint CreateTextureAttachment(GLuint fbo, int width, int height){
    GLuint texture_id;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return texture_id;
}

GLuint CreateFrameBuffer(){
    GLuint fbo_id;
    glGenFramebuffers(1, &fbo_id);
    //glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    //glDrawBuffer(GL_COLOR_ATTACHMENT0);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo_id;
}

GLuint fboId;
GLuint textureId;
GLuint depthId;
bool hasGeneratedFBO = false;

int fbo_width = 800;
int fbo_height = 600;


void DrawMirror(SceneObject& mirrorObj, glm::vec4 mirrorColor, MirrorReflectiveFace reflectiveFace){


    glm::vec4 originalFreeCameraPosition = GetCameraPosition();
    glm::vec4 originalCameraViewVector = GetCameraViewVector();
    CameraMode originalCameraMode = GetCameraMode();

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

    glm::vec4 incidenceVector = mirrorOBB.centro - camera_position;
    float incidenceVectorNorm = norm(incidenceVector);
    incidenceVector = incidenceVector/norm(incidenceVector);


    float t = IntersectionPointRay_OBB(camera_position, incidenceVector, mirrorOBB);
    glm::vec4 intersection_point = camera_position + incidenceVector*t;

    glm::vec4 face_center_point_normal = GetPointInOBBNormal(mirrorOBB, reflective_face_center_point);
    glm::vec4 intersection_point_normal = GetPointInOBBNormal(mirrorOBB, intersection_point);

    if(face_center_point_normal == intersection_point_normal){ //face reflexiva está no campo de visão do jogador
        glm::vec4 reflection_vector = incidenceVector - 2*dot(incidenceVector, intersection_point_normal)*intersection_point_normal;
        reflection_vector = glm::vec4(reflection_vector.x, reflection_vector.y, reflection_vector.z, 0.0f);

        if(!hasGeneratedFBO){
            fboId = CreateFrameBuffer();
            textureId = CreateTextureAttachment(fboId, fbo_width, fbo_height);
            depthId = CreateDepthAttachment(fboId, fbo_width, fbo_height);
            mirrorObj.textureIds.push_back(textureId);
            mirrorObj.activeTexture = 1;
            hasGeneratedFBO = true;
        }

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            exit(0);
        }

        SetCameraPosition(intersection_point-(reflection_vector*incidenceVectorNorm));
        SetCameraMode(CameraMode::FREE);
        SetCameraViewVector(reflection_vector);

        mirrorObj.active = false;
        BindFrameBuffer(fboId, fbo_width, fbo_height);
            glClearColor(0.25f, 0.25f, 0.25f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            DrawCurrentScene();
        UnbindFrameBuffer();

        mirrorObj.active = true;
    }


    SetCameraPosition(originalFreeCameraPosition);
    SetCameraViewVector(originalCameraViewVector);
    SetCameraMode(originalCameraMode);

}
