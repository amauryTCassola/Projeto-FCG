#include "CollisionUtils.h"
#include "objUtils.h"

std::vector<std::string> cameraCollisionsList;
std::vector<std::string> wallCollisionsList;
std::vector<glm::vec4> wallCollisionsNormalsList;
OBB DefineOrientedBoundingBox(SceneObject obj){

    //model = obj.model;

    OBB retorno;

    glm::vec4 max_max_max = obj.model*obj.bbox_max_max_max;
    //glm::vec4 max_max_min = obj.model*obj.bbox_max_max_min;
   // glm::vec4 max_min_max = obj.model*obj.bbox_max_min_max;
    glm::vec4 max_min_min = obj.model*obj.bbox_max_min_min;

    glm::vec4 min_min_min = obj.model*obj.bbox_min_min_min;
    glm::vec4 min_min_max = obj.model*obj.bbox_min_min_max;
    glm::vec4 min_max_min = obj.model*obj.bbox_min_max_min;
    //glm::vec4 min_max_max = obj.model*obj.bbox_min_max_max;

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

    glm::vec4 cVector = glm::vec4(retorno.centro.x, retorno.centro.y, retorno.centro.z, 0.0f);

    retorno.cartesianToOBB = Matrix(
        retorno.eixoU.x   , retorno.eixoU.y   , retorno.eixoU.z   , -dotproduct(retorno.eixoU , cVector) ,
        retorno.eixoV.x   , retorno.eixoV.y   , retorno.eixoV.z   , -dotproduct(retorno.eixoV , cVector) ,
        retorno.eixoW.x   , retorno.eixoW.y   , retorno.eixoW.z   , -dotproduct(retorno.eixoW , cVector) ,
        0.0f , 0.0f , 0.0f , 1.0f);

    retorno.OBBToCartesian = Matrix(
        retorno.eixoU.x   , retorno.eixoV.x   , retorno.eixoW.x   , retorno.centro.x ,
        retorno.eixoU.y   , retorno.eixoV.y   , retorno.eixoW.y   , retorno.centro.y ,
        retorno.eixoU.z   , retorno.eixoV.z   , retorno.eixoW.z   , retorno.centro.z ,
        0.0f , 0.0f , 0.0f , 1.0f);

    return retorno;
}

OBB DefineCameraOBB(){

    OBB retorno;

    retorno.centro = GetCameraPosition();
    retorno.tamanhoU = CAMERA_OBB_SIZE_U;
    retorno.tamanhoV = CAMERA_OBB_SIZE_V;
    retorno.tamanhoW = CAMERA_OBB_SIZE_W;

    retorno.eixoU = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    retorno.eixoV = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    retorno.eixoW = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

    glm::vec4 cVector = glm::vec4(retorno.centro.x, retorno.centro.y, retorno.centro.z, 0.0f);

    retorno.cartesianToOBB = Matrix(
        retorno.eixoU.x   , retorno.eixoU.y   , retorno.eixoU.z   , -dotproduct(retorno.eixoU , cVector) ,
        retorno.eixoV.x   , retorno.eixoV.y   , retorno.eixoV.z   , -dotproduct(retorno.eixoV , cVector) ,
        retorno.eixoW.x   , retorno.eixoW.y   , retorno.eixoW.z   , -dotproduct(retorno.eixoW , cVector) ,
        0.0f , 0.0f , 0.0f , 1.0f);

    retorno.OBBToCartesian = Matrix(
        retorno.eixoU.x   , retorno.eixoV.x   , retorno.eixoW.x   , retorno.centro.x ,
        retorno.eixoU.y   , retorno.eixoV.y   , retorno.eixoW.y   , retorno.centro.y ,
        retorno.eixoU.z   , retorno.eixoV.z   , retorno.eixoW.z   , retorno.centro.z ,
        0.0f , 0.0f , 0.0f , 1.0f);

    return retorno;
}

Sphere DefineSphere(SceneObject obj){

    Sphere retorno;
    OBB aux = DefineOrientedBoundingBox(obj);

    float x = std::max(std::max(aux.tamanhoU, aux.tamanhoV), aux.tamanhoW);

    float raio_global = x/2;
    glm::vec4 centro_global = aux.centro;

    retorno.centro = centro_global;
    retorno.raio = raio_global;
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

        Sphere thisSphere = DefineSphere(obj);
        return IntersectionRay_Sphere(GetCameraPosition(), GetViewVector(), thisSphere);
    }
    else if(obj.thisColliderType == (int)ColliderType::OBB){
        OBB thisOBB = DefineOrientedBoundingBox(obj);
        return IntersectionRay_OBB(GetCameraPosition(), GetViewVector(), thisOBB);
    }
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

    if(colType == MouseCollisionType::MOUSE_OVER && chosenObject.onMouseOver != NULL)
        chosenObject.onMouseOver(currentScene, chosenObjectIndex);
    else if(colType == MouseCollisionType::CLICK && chosenObject.onClick != NULL)
        chosenObject.onClick(currentScene, chosenObjectIndex);
}

std::vector<float> CheckCollision(SceneObject& objA, SceneObject& objB){
    std::vector<float> retorno;

    if(objA.thisColliderType == (int)ColliderType::OBB){

        OBB thisOBB = DefineOrientedBoundingBox(objA);

        if(objB.thisColliderType == (int)ColliderType::OBB){
            OBB thatOBB = DefineOrientedBoundingBox(objB);
            return IntersectionOBB_OBB(thisOBB, thatOBB);
        }
        else if(objB.thisColliderType == (int)ColliderType::SPHERE){
            Sphere thatSphere = DefineSphere(objB);
            return IntersectionOBB_Sphere(thisOBB, thatSphere);
        }

    } else if(objA.thisColliderType == (int)ColliderType::SPHERE){
        Sphere thisSphere = DefineSphere(objA);

        if(objB.thisColliderType == (int)ColliderType::OBB){
            OBB thatOBB = DefineOrientedBoundingBox(objB);
            return IntersectionOBB_Sphere(thatOBB, thisSphere);
        }
        else if(objB.thisColliderType == (int)ColliderType::SPHERE){
            Sphere thatSphere = DefineSphere(objB);
            return IntersectionSphere_Sphere(thisSphere, thatSphere);
        }
    }
    else return retorno;
}

glm::vec4 GetCollisionNormal(glm::vec4 objVelocity, SceneObject wall, glm::vec4 collisionPoint){
    glm::vec4 collisionPointNormal;

    if(wall.thisColliderType == (int)ColliderType::OBB){
        OBB thisOBB = DefineOrientedBoundingBox(wall);

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

    }

    else if(wall.thisColliderType == (int)ColliderType::SPHERE){
        Sphere thisSphere = DefineSphere(wall);
        collisionPointNormal = (collisionPoint - thisSphere.centro)/norm(collisionPoint - thisSphere.centro);
    }

    return collisionPointNormal;
}

std::vector<glm::vec4> GetParallelAndPerpendicularComponentsAndNormal(glm::vec4 objVelocity, SceneObject wall, glm::vec4 collisionPoint){
    glm::vec4 collisionPointNormal = GetCollisionNormal(objVelocity, wall, collisionPoint);
    glm::vec4 perpendicularComponent = ( dotproduct(objVelocity, collisionPointNormal) / dotproduct(collisionPointNormal, collisionPointNormal) ) * collisionPointNormal;
    glm::vec4 parallelComponent = objVelocity - perpendicularComponent;

    return std::vector<glm::vec4>{parallelComponent, perpendicularComponent, collisionPointNormal};
}

void InelasticCollisionWithWall(SceneObject& obj, SceneObject& wall, glm::vec4 collisionPoint){

    std::vector<glm::vec4> components = GetParallelAndPerpendicularComponentsAndNormal(obj.velocity, wall, collisionPoint);

    glm::vec4 parallelComponent = components[0];
    glm::vec4 perpendicularComponent = components[1];

    obj.velocity = parallelComponent + (1 -COEFFICIENT_OF_RESTITUTION)*(-perpendicularComponent);;

    wallCollisionsList.push_back(obj.name);
    wallCollisionsNormalsList.push_back(components[2]);
}

void ElasticCollisionWithWall(SceneObject& obj, SceneObject& wall, glm::vec4 collisionPoint){

    std::vector<glm::vec4> components = GetParallelAndPerpendicularComponentsAndNormal(obj.velocity, wall, collisionPoint);

    glm::vec4 parallelComponent = components[0];
    glm::vec4 perpendicularComponent = components[1];

    glm::vec4 newVelocity = parallelComponent + COEFFICIENT_OF_RESTITUTION*(-perpendicularComponent);

    obj.velocity = newVelocity;

    wallCollisionsList.push_back(obj.name);
    wallCollisionsNormalsList.push_back(components[2]);
}

void InelasticCollision(SceneObject& obj, glm::vec4 velocityObjB){
    float newVelocityX = (obj.velocity.x + velocityObjB.x + (1 - COEFFICIENT_OF_RESTITUTION)*(velocityObjB.x - obj.velocity.x))/2;
    float newVelocityY = (obj.velocity.y + velocityObjB.y + (1 - COEFFICIENT_OF_RESTITUTION)*(velocityObjB.y - obj.velocity.y))/2;
    float newVelocityZ = (obj.velocity.z + velocityObjB.z + (1 - COEFFICIENT_OF_RESTITUTION)*(velocityObjB.z - obj.velocity.z))/2;

    glm::vec4 newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);

    obj.velocity = newVelocity;
}

void ElasticCollision(SceneObject& obj, glm::vec4 velocityObjB){

    float newVelocityX = (obj.velocity.x + velocityObjB.x + COEFFICIENT_OF_RESTITUTION*(velocityObjB.x - obj.velocity.x))/2;
    float newVelocityY = (obj.velocity.y + velocityObjB.y + COEFFICIENT_OF_RESTITUTION*(velocityObjB.y - obj.velocity.y))/2;
    float newVelocityZ = (obj.velocity.z + velocityObjB.z + COEFFICIENT_OF_RESTITUTION*(velocityObjB.z - obj.velocity.z))/2;

    glm::vec4 newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);

    obj.velocity = newVelocity;
}

void ApplyCollisionPhysics(SceneObject& objA, SceneObject& objB, glm::vec4 collisionPoint, bool wasAlreadyColliding, std::vector<SceneObject>& currentScene){

    glm::vec4 velocityA = objA.velocity;
    glm::vec4 velocityB = objB.velocity;

    if(objA.parentIndex == -1 && objB.parentIndex == -1){
        if(!wasAlreadyColliding) {
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
            }else if(objA.thisCollisionType == (int)CollisionType::ELASTIC){
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
        } else{
            if(objB.thisCollisionType == (int)CollisionType::WALL){
                wallCollisionsList.push_back(objA.name);
                wallCollisionsNormalsList.push_back(GetParallelAndPerpendicularComponentsAndNormal(objA.velocity, objB, collisionPoint)[2]);
            } else if(objA.thisCollisionType == (int)CollisionType::WALL){
                wallCollisionsList.push_back(objB.name);
                wallCollisionsNormalsList.push_back(GetParallelAndPerpendicularComponentsAndNormal(objB.velocity, objA, collisionPoint)[2]);
            }
        }
    } else if(objA.parentIndex != -1){
        ApplyCollisionPhysics(currentScene[objA.parentIndex], objB, collisionPoint, wasAlreadyColliding, currentScene);
    } else if(objB.parentIndex != -1){
        ApplyCollisionPhysics(objA, currentScene[objB.parentIndex], collisionPoint, wasAlreadyColliding, currentScene);
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

int IsNameInWallsCollisionsList(std::string _name){
    for(unsigned int i = 0; i< wallCollisionsList.size(); i++){
        if(wallCollisionsList[i].compare(_name) == 0){
            return i;
        }
    }
    return -1;
}

void ElasticCollisionWithCamera(SceneObject& obj, glm::vec4 collisionPoint, glm::vec4 cameraVelocity){
    glm::vec4 newVelocity;

    if(cameraVelocity != glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)){
        float newVelocityX = (obj.velocity.x + cameraVelocity.x + 1*(cameraVelocity.x - obj.velocity.x))/2;
        float newVelocityY = (obj.velocity.y + cameraVelocity.y + 1*(cameraVelocity.y - obj.velocity.y))/2;
        float newVelocityZ = (obj.velocity.z + cameraVelocity.z + 1*(cameraVelocity.z - obj.velocity.z))/2;
        newVelocity = glm::vec4(newVelocityX, newVelocityY, newVelocityZ, 0.0f);
    }
    else{
        glm::vec4 collisionPointNormal;
        OBB thisOBB = DefineCameraOBB();
        glm::vec4 objVelocity = obj.velocity;

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

        glm::vec4 perpendicularComponent = ( dotproduct(objVelocity, collisionPointNormal) / dotproduct(collisionPointNormal, collisionPointNormal) ) * collisionPointNormal;
        glm::vec4 parallelComponent = objVelocity - perpendicularComponent;

        newVelocity = parallelComponent + COEFFICIENT_OF_RESTITUTION*(-perpendicularComponent);
    }

    obj.velocity = newVelocity;
}


SceneObject dummy;

void ApplyCollisionWithCameraPhysics(SceneObject& obj, glm::vec4 collisionPoint, bool wasAlreadyColliding, std::vector<SceneObject>& currentScene, bool isParent = false, SceneObject child = dummy){

        OBB aux;

        if(isParent) aux = DefineOrientedBoundingBox(child);
        else aux = DefineOrientedBoundingBox(obj);

        glm::vec4 vetor = (aux.centro - GetCameraPosition());

        glm::vec4 outroVetor = -vetor;

        glm::vec4 cameraVelocity = GetCameraVelocity();

        std::vector<glm::vec4> components;
        if(isParent) components = GetParallelAndPerpendicularComponentsAndNormal(cameraVelocity, child, collisionPoint);
        else components = GetParallelAndPerpendicularComponentsAndNormal(cameraVelocity, obj, collisionPoint);



        glm::vec4 parallel = components[0];
        glm::vec4 perpendicular = components[1];
        glm::vec4 normal = components[2];
        int indexInWallCollisionsList = IsNameInWallsCollisionsList(obj.name);

        if(obj.parentIndex == -1){
            if(indexInWallCollisionsList != -1){
                glm::vec4 perpendicularToWallNormal = ( dotproduct(cameraVelocity, wallCollisionsNormalsList[indexInWallCollisionsList]) / dotproduct(wallCollisionsNormalsList[indexInWallCollisionsList], wallCollisionsNormalsList[indexInWallCollisionsList]) ) * wallCollisionsNormalsList[indexInWallCollisionsList];
                glm::vec4 parallelToWallNormal = cameraVelocity - perpendicularToWallNormal;
                ElasticCollisionWithCamera(obj, collisionPoint, parallelToWallNormal);
            }
            else{
                if(dotproduct(GetCameraVelocity(), vetor) > 0 || dotproduct(obj.velocity, outroVetor) > 0){
                    if(obj.thisCollisionType == (int)CollisionType::ELASTIC || obj.thisCollisionType == (int)CollisionType::INELASTIC){
                        ElasticCollisionWithCamera(obj, collisionPoint, cameraVelocity);
                    }
                }
            }

        if(dotproduct(cameraVelocity, -normal) > 0)
            SetCameraVelocity(parallel);

    }
    else{
        if(isParent){
            ApplyCollisionWithCameraPhysics(currentScene[obj.parentIndex], collisionPoint, wasAlreadyColliding, currentScene, true, child);
        }
        else ApplyCollisionWithCameraPhysics(currentScene[obj.parentIndex], collisionPoint, wasAlreadyColliding, currentScene, true, obj);
    }
}

std::vector<float> CheckCollisionWithCamera(SceneObject& obj){

    std::vector<float> retorno;

    OBB cameraOBB = DefineCameraOBB();
    if(obj.thisColliderType == (int)ColliderType::SPHERE){
        Sphere thisSphere = DefineSphere(obj);
        return IntersectionOBB_Sphere(cameraOBB, thisSphere);
    }
    else if(obj.thisColliderType == (int)ColliderType::OBB){
        OBB thisOBB = DefineOrientedBoundingBox(obj);
        return IntersectionOBB_OBB(thisOBB, cameraOBB);
    }
    return retorno;
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

    bool alreadyColliding = false;


    for(unsigned int i = 0; i < colliders.size(); i++){
        std::vector<std::string> newCollisionsList;
        for(unsigned int j = i+1; j < colliders.size(); j++){
              if(!IsNameInCollisionsList(currentScene[i].childrenNames, currentScene[j].name)){
                std::vector<float> collisionPoint = CheckCollision(currentScene[i], currentScene[j]);

                if(collisionPoint.size() == 4){ //houve colisão
                        glm::vec4 vec4CollisionPoint = glm::vec4(collisionPoint[0], collisionPoint[1], collisionPoint[2], collisionPoint[3]);

                        newCollisionsList.push_back(currentScene[j].name);

                        alreadyColliding = IsNameInCollisionsList(currentScene[i].collisionsList, currentScene[j].name);

                        if(!alreadyColliding){
                            if(currentScene[i].onCollision != NULL)currentScene[i].onCollision(currentScene, i, j);
                            if(currentScene[j].onCollision != NULL)currentScene[j].onCollision(currentScene, j, i);
                        }

                            ApplyCollisionPhysics(currentScene[i], currentScene[j], vec4CollisionPoint, alreadyColliding, currentScene);
                }
            }
        }
        currentScene[i].collisionsList = newCollisionsList;
        newCollisionsList.clear();
    }

    std::vector<std::string> newCameraCollisionsList;

    if(GetCameraMode() == CameraMode::FREE){
        for(unsigned int i = 0; i < colliders.size(); i++){
            if(GetDistanceFromCamera(currentScene[i]) < MAX_DISTANCE_FROM_CAMERA){
                std::vector<float> collisionWithCameraPoint = CheckCollisionWithCamera(currentScene[i]);
                if(collisionWithCameraPoint.size() == 4){ //houve colisão com a câmera
                    glm::vec4 vec4CollisionPoint = glm::vec4(collisionWithCameraPoint[0], collisionWithCameraPoint[1], collisionWithCameraPoint[2], collisionWithCameraPoint[3]);
                    newCameraCollisionsList.push_back(currentScene[i].name);
                   // if(!IsNameInCollisionsList(cameraCollisionsList, currentScene[i].name)){
                        if(currentScene[i].onCollision != NULL)currentScene[i].onCollision(currentScene, i, COLLISION_WITH_CAMERA_CODE);
                        ApplyCollisionWithCameraPhysics(currentScene[i], vec4CollisionPoint, IsNameInCollisionsList(cameraCollisionsList, currentScene[i].name), currentScene);
                   // }
                }
            }
        }
        cameraCollisionsList = newCameraCollisionsList;
        wallCollisionsList.clear();
        wallCollisionsNormalsList.clear();
    }

}
