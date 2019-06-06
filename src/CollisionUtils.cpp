#include "CollisionUtils.h"
#include "objUtils.h"



OBB DefineOrientedBoundingBox(SceneObject obj){

    //model = obj.model;

    OBB retorno;

    glm::vec4 max_max_max = obj.model*obj.bbox_max_max_max;
    glm::vec4 max_max_min = obj.model*obj.bbox_max_max_min;
    glm::vec4 max_min_max = obj.model*obj.bbox_max_min_max;
    glm::vec4 max_min_min = obj.model*obj.bbox_max_min_min;

    glm::vec4 min_min_min = obj.model*obj.bbox_min_min_min;
    glm::vec4 min_min_max = obj.model*obj.bbox_min_min_max;
    glm::vec4 min_max_min = obj.model*obj.bbox_min_max_min;
    glm::vec4 min_max_max = obj.model*obj.bbox_min_max_max;

    glm::vec4 eixoU = max_min_min - min_min_min;
    glm::vec4 eixoV = min_max_min - min_min_min;
    glm::vec4 eixoW = min_min_max - min_min_min;

    retorno.tamanhoU = norm(eixoU);
    retorno.tamanhoV = norm(eixoV);
    retorno.tamanhoW = norm(eixoW);

    retorno.eixoU = eixoU/norm(eixoU);
    retorno.eixoV = eixoV/norm(eixoV);
    retorno.eixoW = eixoW/norm(eixoW);

    retorno.centro = (min_min_min+max_max_max)*0.5f;

    /*retorno.c = min_min_min;

    glm::vec4 cVector = glm::vec4(retorno.c.x, retorno.c.y, retorno.c.z, 0.0f);

    retorno.cartesianToOBB = Matrix(
        retorno.eixoU.x   , retorno.eixoU.y   , retorno.eixoU.z   , -dotproduct(retorno.eixoU , cVector) ,
        retorno.eixoV.x   , retorno.eixoV.y   , retorno.eixoV.z   , -dotproduct(retorno.eixoV , cVector) ,
        retorno.eixoW.x   , retorno.eixoW.y   , retorno.eixoW.z   , -dotproduct(retorno.eixoW , cVector) ,
        0.0f , 0.0f , 0.0f , 1.0f);

    retorno.OBBToCartesian = Matrix(
        retorno.eixoU.x   , retorno.eixoV.x   , retorno.eixoW.x   , retorno.c.x ,
        retorno.eixoU.y   , retorno.eixoV.y   , retorno.eixoW.y   , retorno.c.y ,
        retorno.eixoU.z   , retorno.eixoV.z   , retorno.eixoW.z   , retorno.c.z ,
        0.0f , 0.0f , 0.0f , 1.0f);*/


        //printf("\ncentro: (%f %f %f)", cu);

    return retorno;
}

OBB DefineCameraOBB(){

    OBB cameraOBB;
    /*glm::vec4 cameraPosition = GetCameraPosition();

    cameraOBB.c = glm::vec4(cameraPosition.x-CAMERA_OBB_SIZE_U/2, cameraPosition.y-CAMERA_OBB_SIZE_V/2, cameraPosition.z-CAMERA_OBB_SIZE_W/2, 1.0f);
    cameraOBB.eixoU = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    cameraOBB.eixoV = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    cameraOBB.eixoW = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

    cameraOBB.tamanhoU = CAMERA_OBB_SIZE_U;
    cameraOBB.tamanhoV = CAMERA_OBB_SIZE_V;
    cameraOBB.tamanhoW = CAMERA_OBB_SIZE_W;*/

    return cameraOBB;
}

Sphere DefineSphere(SceneObject obj){

    Sphere retorno;
    OBB aux = DefineOrientedBoundingBox(obj);

    /*glm::vec4 centro_local = (obj.bbox_min_min_min + obj.bbox_max_max_max)/2.0f;
    glm::vec4 centro_global = obj.model * centro_local;*/

    float x = std::max(std::max(aux.tamanhoU, aux.tamanhoV), aux.tamanhoW);

    float raio_global = x/2;
    glm::vec4 centro_global = aux.centro;



    /*glm::vec4 maxXEsfera = glm::vec4(obj.bbox_max.x, 0.0f, 0.0f, 1.0f);
    glm::vec4 raio_local_vec = (maxXEsfera - centro_local);
    glm::vec4 raio_global_vec = obj.model * raio_local_vec;
    float raio_global = norm(raio_global_vec);*/

    retorno.centro = centro_global;
    retorno.raio = raio_global;

    //printf("\ncentro (%2.f, %2.f, %2.f) raio: %2.f", centro_global.x, centro_global.y, centro_global.z, raio_global);

    return retorno;

}

Cylinder DefineCylinder(SceneObject obj){

    Cylinder retorno;
    Sphere auxSphere = DefineSphere(obj);
    OBB auxOBB = DefineOrientedBoundingBox(obj);
    retorno.raio = auxSphere.raio;
    retorno.eixo = auxOBB.eixoV;
    retorno.centro = auxSphere.centro;

    return retorno;

}


glm::vec4 Projecao(glm::vec4 vecA, glm::vec4 vecB){ //de A em B
    return (dotproduct(vecA, vecB)*vecB)/(norm(vecB)*norm(vecB));
}

float GetDistanceFromCamera(SceneObject obj){

    glm::vec4 bbox_min_world = obj.model * obj.bbox_min_min_min;
    glm::vec4 bbox_max_world = obj.model * obj.bbox_max_max_max;

    glm::vec4 centroObj = (bbox_min_world + bbox_max_world) / 2.0f;

    return norm(centroObj - GetCameraPosition());
}

bool TestRayIntersection(SceneObject obj){
    if(obj.thisColliderType == (int)ColliderType::SPHERE){

        Sphere a = DefineSphere(obj);
        return IntersectionRaySphere(GetCameraPosition(), GetViewVector(), a.centro, a.raio);
    }
    else if(obj.thisColliderType == (int)ColliderType::OBB){
        OBB a = DefineOrientedBoundingBox(obj);

        glm::vec4 min_global = a.centro - a.eixoU*0.5f - a.eixoV*0.5f - a.eixoW*0.5f;
        glm::vec4 max_global = a.centro + a.eixoU*0.5f + a.eixoV*0.5f + a.eixoW*0.5f;


        /*glm::vec4 max_local = glm::vec4(a.tamanhoU, a.tamanhoV, a.tamanhoW, 1.0f);
        glm::vec4 min_local = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec4 pontoA = a.OBBToCartesian*max_local;
        glm::vec4 pontoB = a.OBBToCartesian*min_local;

        glm::vec4 max_global = glm::vec4(std::max(pontoA.x, pontoB.x), std::max(pontoA.y, pontoB.y), std::max(pontoA.z, pontoB.z), 1.0f);
        glm::vec4 min_global = glm::vec4(std::min(pontoA.x, pontoB.x), std::min(pontoA.y, pontoB.y), std::min(pontoA.z, pontoB.z), 1.0f);*/

       return IntersectionRayCube(GetCameraPosition(), GetViewVector(), min_global, max_global);
    }
    /*
    else if(obj.thisColliderType == ColliderType::CYLINDER){

    }
    */
    else return false;
}

void TestMouseCollision(MouseCollisionType colType, std::vector<SceneObject>& currentScene){

    std::vector<SceneObject> mouseOverCandidates;
    std::vector<int> candidatesIndices;
    SceneObject chosenObject;
    int chosenObjectIndex;
    float chosenObjectDistance = -1.0f;

    for(unsigned int i = 0; i < currentScene.size(); i++){
        if(currentScene[i].active
           && GetDistanceFromCamera(currentScene[i]) < MAX_DISTANCE_FROM_CAMERA
           && currentScene[i].thisColliderType != (int)ColliderType::NONE){
                mouseOverCandidates.push_back(currentScene[i]);
                candidatesIndices.push_back(i);
           }
    }

    if(mouseOverCandidates.size() == 0) return;

    for(unsigned int i = 0; i < mouseOverCandidates.size(); i++){
        if(TestRayIntersection(mouseOverCandidates[i])){
            if(chosenObjectDistance == -1 || GetDistanceFromCamera(mouseOverCandidates[i]) < chosenObjectDistance){
                chosenObject = mouseOverCandidates[i];
                chosenObjectDistance = GetDistanceFromCamera(mouseOverCandidates[i]);
                chosenObjectIndex = candidatesIndices[i];
            }
        }
    }

    if(chosenObjectDistance == -1.0f) return;

    if(colType == MouseCollisionType::MOUSE_OVER)
        chosenObject.onMouseOver(currentScene, chosenObjectIndex);
    else if(colType == MouseCollisionType::CLICK)
        chosenObject.onClick(currentScene, chosenObjectIndex);
}

std::vector<float> CheckCollision(SceneObject& objA, SceneObject& objB){
    std::vector<float> retorno;

    if(objA.thisColliderType == (int)ColliderType::OBB){

            OBB thisOBB = DefineOrientedBoundingBox(objA);
            Sphere thisSphere = DefineSphere(objB);
            printf("BBBBBBBB");
        switch((ColliderType)objB.thisColliderType){
            case ColliderType::OBB:
                //retorno = IntersectionAABB_AABB();
                return retorno;
            break;
            case ColliderType::CYLINDER:
                //retorno = IntersectionAABB_Cylinder();
                return retorno;
            break;
            case ColliderType::SPHERE:
                return IntersectionOBB_Sphere(thisOBB, thisSphere);
            break;
            default:
                return retorno;
        }
    } else if(objA.thisColliderType == (int)ColliderType::CYLINDER){
        switch((ColliderType)objB.thisColliderType){
            case ColliderType::OBB:
                //retorno = IntersectionAABB_Cylinder();
                return retorno;
            break;
            case ColliderType::CYLINDER:
                //retorno = IntersectionCylinder_Cylinder();
                return retorno;
            break;
            case ColliderType::SPHERE:
                //retorno = IntersectionCylinder_Sphere();
                return retorno;
            break;
            default:
                return retorno;
        }
    } else if(objA.thisColliderType == (int)ColliderType::SPHERE){

        OBB thisOBB = DefineOrientedBoundingBox(objB);
        Sphere thisSphere = DefineSphere(objA);


        switch((ColliderType)objB.thisColliderType){
            case ColliderType::OBB:
                return IntersectionOBB_Sphere(thisOBB, thisSphere);
            break;
            case ColliderType::CYLINDER:
                //retorno = IntersectionCylinder_Sphere();
                return retorno;
            break;
            case ColliderType::SPHERE:
                //retorno = IntersectionSphere_Sphere();
                return retorno;
            break;
            default:
                return retorno;
        }
    }
    else return retorno;
}

std::vector<glm::vec4> GetParallelAndPerpendicularComponents(glm::vec4 objVelocity, SceneObject wall, glm::vec4 collisionPoint){
    glm::vec4 collisionPointNormal;

    if(wall.thisColliderType == (int)ColliderType::OBB){
        OBB thisOBB = DefineOrientedBoundingBox(wall);

        glm::vec4 collisionPInOBBCoordinates = thisOBB.cartesianToOBB * collisionPoint;

        glm::vec4 boxCenter = glm::vec4(thisOBB.tamanhoU/2, thisOBB.tamanhoV/2, thisOBB.tamanhoW/2, 1.0f);

        glm::vec4 vectorCenterToPoint = collisionPInOBBCoordinates - boxCenter;

        float divX = boxCenter.x;
        float divY = boxCenter.y;
        float divZ = boxCenter.w;

        int normalX = (int)(vectorCenterToPoint.x/divX);
        int normalY = (int)(vectorCenterToPoint.y/divY);
        int normalZ = (int)(vectorCenterToPoint.z/divZ);

        glm::vec4 normalInOBBCoordinates = glm::vec4((float)normalX, (float)normalY, (float)normalZ, 0.0f);

        collisionPointNormal = thisOBB.OBBToCartesian * normalInOBBCoordinates;
    }

    else if(wall.thisColliderType == (int)ColliderType::SPHERE){
        Sphere thisSphere = DefineSphere(wall);
        collisionPointNormal = (collisionPoint - thisSphere.centro)/norm(collisionPoint - thisSphere.centro);
    }

    else if(wall.thisColliderType == (int)ColliderType::CYLINDER){
        Cylinder thisCylinder = DefineCylinder(wall);
        glm::vec4 v = collisionPoint - thisCylinder.centro;
        glm::vec4 u = Projecao(v, thisCylinder.eixo);
        glm::vec4 q = thisCylinder.centro + u;
        collisionPointNormal = collisionPoint - q;
    }

    glm::vec4 perpendicularComponent = (dotproduct(objVelocity, collisionPointNormal) / dotproduct(collisionPointNormal, collisionPointNormal)) * collisionPointNormal;
    glm::vec4 parallelComponent = objVelocity - perpendicularComponent;

    return std::vector<glm::vec4>{parallelComponent, perpendicularComponent};
}

void InelasticCollisionWithWall(SceneObject& obj, SceneObject& wall, glm::vec4 collisionPoint){

    std::vector<glm::vec4> components = GetParallelAndPerpendicularComponents(obj.velocity, wall, collisionPoint);

    glm::vec4 parallelComponent = components[0];
    glm::vec4 perpendicularComponent = components[1];


    MoveObject(-(perpendicularComponent/norm(perpendicularComponent)), obj);
    obj.velocity = parallelComponent;
}

void ElasticCollisionWithWall(SceneObject& obj, SceneObject& wall, glm::vec4 collisionPoint){

    std::vector<glm::vec4> components = GetParallelAndPerpendicularComponents(obj.velocity, wall, collisionPoint);

    glm::vec4 parallelComponent = components[0];
    glm::vec4 perpendicularComponent = components[1];

    glm::vec4 newVelocity = parallelComponent - COEFFICIENT_OF_RESTITUTION * perpendicularComponent;
    newVelocity = newVelocity * COEFFICIENT_OF_RESTITUTION;

    obj.velocity = newVelocity;
}

void InelasticCollision(SceneObject& obj, glm::vec4 velocityObjB){
    float newVelocityX = (obj.velocity.x + velocityObjB.x + 0*(velocityObjB.x - obj.velocity.x))/2;
    float newVelocityY = (obj.velocity.y + velocityObjB.y + 0*(velocityObjB.y - obj.velocity.y))/2;
    float newVelocityZ = (obj.velocity.z + velocityObjB.z + 0*(velocityObjB.z - obj.velocity.z))/2;

    glm::vec4 newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);

    obj.velocity = newVelocity;
}

void ElasticCollision(SceneObject& obj, glm::vec4 velocityObjB){

    float newVelocityX = (obj.velocity.x + velocityObjB.x + 1*(velocityObjB.x - obj.velocity.x))/2;
    float newVelocityY = (obj.velocity.y + velocityObjB.y + 1*(velocityObjB.y - obj.velocity.y))/2;
    float newVelocityZ = (obj.velocity.z + velocityObjB.z + 1*(velocityObjB.z - obj.velocity.z))/2;

    glm::vec4 newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);

    obj.velocity = newVelocity;
}

void ApplyCollisionPhysics(SceneObject& objA, SceneObject& objB, glm::vec4 collisionPoint, bool wasAlreadyColliding){

    glm::vec4 velocityA = objA.velocity;
    glm::vec4 velocityB = objB.velocity;

    if(!wasAlreadyColliding){
        if(objB.thisCollisionType == (int)CollisionType::WALL){
            if(objA.thisCollisionType == (int)CollisionType::ELASTIC){
                ElasticCollisionWithWall(objA, objB, collisionPoint);
            }
            else{
                InelasticCollisionWithWall(objA, objB, collisionPoint);
            }
        } else if(objA.thisCollisionType == (int)CollisionType::WALL){
            if(objB.thisCollisionType == (int)CollisionType::ELASTIC){
                ElasticCollisionWithWall(objB, objA, collisionPoint);
            }
            else{
                InelasticCollisionWithWall(objB, objA, collisionPoint);
            }
        } else {

                if(objA.thisCollisionType == (int)CollisionType::ELASTIC){
                        ElasticCollision(objA, velocityB);
                }
                else if(objA.thisCollisionType == (int)CollisionType::INELASTIC){
                        InelasticCollision(objA, velocityB);
                }


                if(objB.thisCollisionType == (int)CollisionType::ELASTIC){
                        ElasticCollision(objB, velocityA);
                }
                else if(objB.thisCollisionType == (int)CollisionType::INELASTIC){
                        InelasticCollision(objB, velocityA);
                }
            }
        }
    else{
            //exit(0);
        //printf("JÁ ESTAVA COLIDINDO");
    }
}

bool IsNameInCollisionsList(std::vector<std::string> _collisionsList, std::string _name){
    for(unsigned int i = 0; i< _collisionsList.size(); i++){
        if(_collisionsList[i].compare(_name) == 0){
            return true;
        }
    }
    return false;
}

void ElasticCollisionWithCamera(SceneObject& obj){

    glm::vec4 cameraVelocity = GetCameraVelocity();

    float newVelocityX = (obj.velocity.x + cameraVelocity.x + COEFFICIENT_OF_RESTITUTION*(cameraVelocity.x - obj.velocity.x))/2;
    float newVelocityY = (obj.velocity.y + cameraVelocity.y + COEFFICIENT_OF_RESTITUTION*(cameraVelocity.y - obj.velocity.y))/2;
    float newVelocityZ = (obj.velocity.z + cameraVelocity.z + COEFFICIENT_OF_RESTITUTION*(cameraVelocity.z - obj.velocity.z))/2;

    glm::vec4 newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);

    obj.velocity = newVelocity;
}

void InelasticCollisionWithCamera(SceneObject& obj){
    obj.velocity = GetCameraVelocity();
}

void CameraCollisionWithWall(SceneObject wall, glm::vec4 collisionPoint){
    std::vector<glm::vec4> components = GetParallelAndPerpendicularComponents(GetCameraVelocity(), wall, collisionPoint);

    glm::vec4 parallelComponent = components[0];
    glm::vec4 perpendicularComponent = components[1];


    MoveCameraByVector(-(perpendicularComponent/norm(perpendicularComponent)));
    SetCameraVelocity(parallelComponent);
}

void ApplyCollisionWithCameraPhysics(SceneObject& obj, glm::vec4 collisionPoint){
    if(obj.thisCollisionType == (int)CollisionType::WALL){
        CameraCollisionWithWall(obj, collisionPoint);
    }
    else if(obj.thisCollisionType == (int)CollisionType::ELASTIC){
        ElasticCollisionWithCamera(obj);
    }
    else if(obj.thisCollisionType == (int)CollisionType::INELASTIC){
        InelasticCollisionWithCamera(obj);
    }
}

std::vector<float> CheckCollisionWithCamera(SceneObject& obj){

    std::vector<float> retorno;

    OBB cameraOBB = DefineCameraOBB();
    Sphere thisSphere;

    if(obj.thisColliderType == (int)ColliderType::SPHERE)
    thisSphere = DefineSphere(obj);

    switch((ColliderType)obj.thisColliderType){
        case ColliderType::SPHERE:
            return IntersectionOBB_Sphere(cameraOBB, thisSphere);
        break;
        case ColliderType::CYLINDER:
            //retorno = IntersectionAABB_Cylinder();
            return retorno;
        break;
        case ColliderType::OBB:
            //retorno = IntersectionAABB_Sphere();
            return retorno;
        break;
        default:
            return retorno;
    }
}

void TestCollisions(std::vector<SceneObject>& currentScene){
    std::vector<SceneObject> colliders;

    for(unsigned int i = 0; i < currentScene.size(); i++){
        if(currentScene[i].active
           && currentScene[i].thisColliderType != (int)ColliderType::NONE){
                colliders.push_back(currentScene[i]);
           }
    }

    if(colliders.size() == 0) return;


    /*for(unsigned int i = 0; i < colliders.size(); i++){

        //if(GetDistanceFromCamera(currentScene[i]) < MAX_DISTANCE_FROM_CAMERA){
            std::vector<float> collisionWithCameraPoint = CheckCollisionWithCamera(currentScene[i]);

            if(collisionWithCameraPoint.size() == 4){ //houve colisão com a câmera
                glm::vec4 vec4CollisionPoint = glm::vec4(collisionWithCameraPoint[0], collisionWithCameraPoint[1], collisionWithCameraPoint[2], collisionWithCameraPoint[3]);

                if(currentScene[i].onCollision != NULL)currentScene[i].onCollision(currentScene, i, COLLISION_WITH_CAMERA_CODE);

                //printf("COLIDIU COM A CAMERA AAAAAAAAAAAAAA");
                //ApplyCollisionWithCameraPhysics(currentScene[i], vec4CollisionPoint);
            }
        //}

    }*/


    for(unsigned int i = 0; i < colliders.size(); i++){
        std::vector<std::string> newCollisionsList;
        for(unsigned int j = i+1; j < colliders.size(); j++){
            std::vector<float> collisionPoint = CheckCollision(currentScene[i], currentScene[j]);

            if(collisionPoint.size() == 4){ //houve colisão
                    glm::vec4 vec4CollisionPoint = glm::vec4(collisionPoint[0], collisionPoint[1], collisionPoint[2], collisionPoint[3]);

                    bool wasAlreadyColliding = IsNameInCollisionsList(currentScene[i].collisionsList, currentScene[j].name);

                    newCollisionsList.push_back(currentScene[j].name);

                    if(!wasAlreadyColliding){
                        if(currentScene[i].onCollision != NULL)currentScene[i].onCollision(currentScene, i, j);
                        if(currentScene[j].onCollision != NULL)currentScene[j].onCollision(currentScene, j, i);

                        ApplyCollisionPhysics(currentScene[i], currentScene[j], vec4CollisionPoint, wasAlreadyColliding);
                    }
            }
        }
        currentScene[i].collisionsList = newCollisionsList;
        newCollisionsList.clear();
    }

}
