#include "SpriteManager.h"
#include <iostream>

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    sf::Texture td(texturePath);
    mTextureDataMap[texturePath] = td;
    //std::cout << "\nSourceRect: "<< sourceRect << "\n";
    sprites.push_back(SpriteData(mTextureDataMap[texturePath],sourceRect));
    //assuming there is no deletion in sprites
    SpriteID id = sprites.size() - 1; 
    return id;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    return sprites[id].sprite;
}

}  // namespace ECSEngine
