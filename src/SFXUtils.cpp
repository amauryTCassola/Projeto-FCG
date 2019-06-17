#include "SFXUtils.h"
#include <irrKlang.h>

irrklang::ISoundEngine* engine;
bool isEngineStarted = false;
void InitSFX(){
    engine = irrklang::createIrrKlangDevice();
    isEngineStarted = true;
}

void PlaySound(std::string filename, bool loop, float volume){
    if(!isEngineStarted)
        InitSFX();

    volume = std::max(0.0f, std::min(1.0f, volume));
    irrklang::ISound* snd = engine->play2D(filename.c_str(), loop, true);
    snd->setVolume(volume);
    snd->setIsPaused(false);

}
