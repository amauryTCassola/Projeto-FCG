#include "Scene0Functions.h"
std::function<void(std::vector<SceneObject>&, int callerIndex)> FunctionMapping(std::string functionName){

    if(functionName.compare("SphereOnClick") == 0) return SphereOnClick;
    if(functionName.compare("SphereOnMouseOver") == 0) return SphereOnMouseOver;
    if(functionName.compare("SphereOnMove") == 0) return SphereOnMove;
    if(functionName.compare("SphereChildOnMove") == 0) return SphereChildOnMove;
    if(functionName.compare("RabbitOnClick") == 0) return RabbitOnClick;
    if(functionName.compare("SphereChildUpdate") == 0) return SphereChildUpdate;
    if(functionName.compare("MirrorUpdate") == 0) return MirrorUpdate;

    return NULL;
}

std::function<void(std::vector<SceneObject>&, int, int)> CollisionFunctionMapping(std::string functionName){
    return NULL;
}
