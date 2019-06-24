#include "Scene0Functions.h"
std::function<void(std::vector<SceneObject>&, int callerIndex)> FunctionMapping(std::string functionName){

    if(functionName.compare("SphereOnClick") == 0) return SphereOnClick;
    else if(functionName.compare("SphereOnMouseOver") == 0) return SphereOnMouseOver;
    else if(functionName.compare("SphereOnMove") == 0) return SphereOnMove;
    else if(functionName.compare("SphereChildOnMove") == 0) return SphereChildOnMove;
    else if(functionName.compare("RabbitOnClick") == 0) return RabbitOnClick;
    else if(functionName.compare("SphereChildUpdate") == 0) return SphereChildUpdate;
    else if(functionName.compare("MirrorUpdate") == 0) return MirrorUpdate;
    else if(functionName.compare("DescricaoDummy") == 0) return DescricaoDummy;
    else if(functionName.compare("GavetaOnClick") == 0) return GavetaOnClick;
    else if(functionName.compare("GavetaUpdate") == 0) return GavetaUpdate;
    else if(functionName.compare("GavetaOnMouseOver") == 0) return GavetaOnMouseOver;
    else if(functionName.compare("VidroDummyOnMouseOver") == 0) return VidroDummyOnMouseOver;
    else if(functionName.compare("VidroDummyOnClick") == 0) return VidroDummyOnClick;
    else if(functionName.compare("AbstrataDummyOnMouseOver") == 0) return AbstrataDummyOnMouseOver;
    else if(functionName.compare("DescricaoVenus") == 0) return DescricaoVenus;
    else if(functionName.compare("DescricaoGourard") == 0) return DescricaoGourard;
    else if(functionName.compare("DescricaoBunny") == 0) return DescricaoBunny;
    else if(functionName.compare("DescricaoChest") == 0) return DescricaoChest;
    else if(functionName.compare("LightningGeneratorUpdate") == 0) return LightningGeneratorUpdate;
    else if(functionName.compare("DescricaoBau") == 0) return DescricaoBau;
    else if(functionName.compare("DescricaoStarryNight") == 0) return DescricaoStarryNight;
    else if(functionName.compare("AbreBau") == 0) return AbreBau;
    else if(functionName.compare("AnimacaoBau") == 0) return AnimacaoBau;
    else if(functionName.compare("ChaveOnMouseOver") == 0) return ChaveOnMouseOver;
    else if(functionName.compare("PortaOnMouseOver") == 0) return PortaOnMouseOver;
    else if(functionName.compare("PortaOnClick") == 0) return PortaOnClick;
    else if(functionName.compare("ChaveOnClick") == 0) return ChaveOnClick;
    else if(functionName.compare("DummyHeadUpdate") == 0) return DummyHeadUpdate;
    else if(functionName.compare("RotateSphereUpdate") == 0) return RotateSphereUpdate;



    return NULL;
}

std::function<void(std::vector<SceneObject>&, int, int)> CollisionFunctionMapping(std::string functionName){
    return NULL;
}
