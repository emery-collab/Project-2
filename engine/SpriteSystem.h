#pragma once

#include <bitset>
#include <unordered_map>

#include "CameraComponent.h"
#include "EntityManager.h"
#include "MathUtil.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& em = engine->GetEntityManager();
        auto& spriteManager = engine->GetSpriteManager();
        auto& windowManager = engine->GetWindowManager();

        auto window = windowManager.GetWindow();

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id)) {
                continue;
            }

            if (!em.template HasComponent<SpriteComponent>(id)) {
                continue;
            }

            auto& spr = em.template GetComponent<SpriteComponent>(id);

            Point2D position = spr.bounds.topLeft;

            // World-space sprites need a location component.
            if (spr.worldSpace) {
                if (!em.template HasComponent<LocationComponent>(id)) {
                    continue;
                }

                auto& loc = em.template GetComponent<LocationComponent>(id);
                position = windowManager.WorldToWindow(loc.loc + spr.bounds.topLeft);
            }

            // Each entity may share a sprite ID, so update its position
            // immediately before drawing it.
            sf::Sprite& sprite = spriteManager.GetSprite(spr.textureID);
            sprite.setPosition({position.x, position.y});

            window->draw(sprite);
        }

        window->display();

        return kContinue;
    }
};

}  // namespace ECSEngine
