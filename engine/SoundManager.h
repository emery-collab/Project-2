#pragma once

#include <unordered_map>
#include <iostream>

#include <SFML/Audio.hpp>

namespace ECSEngine {

class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager() = default;
    void RegisterSound(const std::string& soundPath, const std::string& soundName);
    sf::Sound& GetSound(const std::string& soundName);

private:
    struct SoundData {
        sf::SoundBuffer buffer;
        sf::Sound sound;
        SoundData(const std::string& path)
        : buffer(), 
          sound(buffer)   
        {
            // Load the file into the buffer
            if (!buffer.loadFromFile(path)) {
                std::cerr << "Error loading audio file!" << std::endl;
                throw std::out_of_range("No sound filepath with name: " + path + ".\n" );
            }

            //sound.setBuffer(buffer);
        }
    };
    
    //https://stackoverflow.com/questions/70310112/how-to-make-a-vector-of-sfsound
    std::unordered_map<std::string,std::shared_ptr<SoundData>> mSounds;
    // using SoundPtr = std::shared_ptr<sf::Sound>;
    // std::unordered_map<std::string, SoundPtr > mSoundPtr;

};

}  // namespace ECSEngine
