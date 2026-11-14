#pragma once

#include <unordered_map>
#include <vector>
#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;

// Owns textures (unordered_map, stable addresses) and sprites (vector; SpriteID = index).
class SpriteManager {
public:
    SpriteManager() = default;
    ~SpriteManager() = default;

    // Makes a sprite from the sourceRect part of the texture. The ID stays valid for the manager's lifetime.
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);

    // id isn't checked. Reference is only valid until the next RegisterTexture (vector may move).
    sf::Sprite& GetSprite(SpriteID id);

    struct SpriteData{
        sf::Texture* texData;
        Rect rectangle;
        sf::Sprite sprite;
        SpriteData(sf::Texture& t,const Rect& r)
        : texData(&t),rectangle(r),sprite(*texData){
            sprite.setTextureRect(sf::IntRect({(int)r.topLeft.x, (int)r.topLeft.y}, {(int)r.width, (int)r.height}));
        }
    };

private:
    std::unordered_map<std::string, sf::Texture> mTextureDataMap;
    std::vector<SpriteData> sprites;
};

}  // namespace ECSEngine
