#include "Scene0Functions.h"
std::function<void(std::vector<SceneObject>&, int callerIndex)> FunctionMapping(std::string functionName){

    if(functionName.compare("SphereOnClick") == 0) return SphereOnClick;
    if(functionName.compare("SphereOnMouseOver") == 0) return SphereOnMouseOver;
    if(functionName.compare("SphereOnMove") == 0) return SphereOnMove;
    if(functionName.compare("SphereChildOnMove") == 0) return SphereChildOnMove;
    if(functionName.compare("RabbitOnClick") == 0) return RabbitOnClick;
    if(functionName.compare("SphereChildUpdate") == 0) return SphereChildUpdate;
    if(functionName.compare("MirrorUpdate") == 0) return MirrorUpdate;
    if(functionName.compare("DescricaoDummy") == 0) return DescricaoDummy;
    if(functionName.compare("GavetaOnClick") == 0) return GavetaOnClick;
    if(functionName.compare("GavetaUpdate") == 0) return GavetaUpdate;
    if(functionName.compare("GavetaOnMouseOver") == 0) return GavetaOnMouseOver;
    if(functionName.compare("VidroDummyOnMouseOver") == 0) return VidroDummyOnMouseOver;
    if(functionName.compare("VidroDummyOnClick") == 0) return VidroDummyOnClick;
    if(functionName.compare("AbstrataDummyOnMouseOver") == 0) return AbstrataDummyOnMouseOver;
    if(functionName.compare("DescricaoVenus") == 0) return DescricaoVenus;
    if(functionName.compare("DescricaoGourard") == 0) return DescricaoGourard;
    if(functionName.compare("DescricaoBunny") == 0) return DescricaoBunny;
    if(functionName.compare("DescricaoChest") == 0) return DescricaoChest;
    if(functionName.compare("LightningGeneratorUpdate") == 0) return LightningGeneratorUpdate;
    if(functionName.compare("DescricaoBau") == 0) return DescricaoBau;
    if(functionName.compare("DescricaoStarryNight") == 0) return DescricaoStarryNight;
    if(functionName.compare("AbreBau") == 0) return AbreBau;
    if(functionName.compare("AnimacaoBau") == 0) return AnimacaoBau;
    if(functionName.compare("ChaveOnMouseOver") == 0) return ChaveOnMouseOver;



    return NULL;
}

std::function<void(std::vector<SceneObject>&, int, int)> CollisionFunctionMapping(std::string functionName){
    return NULL;
}
