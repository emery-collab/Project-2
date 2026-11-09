#include "SoundManager.h"
#include <iostream>
namespace ECSEngine {

void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    SoundData sd(soundPath);
    std::shared_ptr<SoundData> soundPtr = std::make_shared<SoundData>(soundPath);
    mSounds[soundName] = soundPtr;
    
}

sf::Sound& SoundManager::GetSound(const std::string& soundName) {
    SoundData& s = *mSounds.at(soundName);

    return s.sound;
}

}  // namespace ECSEngine
