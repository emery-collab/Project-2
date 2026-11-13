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
    static Rect GetWorldBox(const LocationComponent& location,
                            const CollisionComponent& collision) {
        return Rect(
            Point2D(location.loc.x + collision.collisionBox.topLeft.x,
                    location.loc.y + collision.collisionBox.topLeft.y),
            collision.collisionBox.width,
            collision.collisionBox.height);
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& em = engine->GetEntityManager();
        auto& spriteManager = engine->GetSpriteManager();
        auto& windowManager = engine->GetWindowManager();

        auto window = windowManager.GetWindow();

        // Clear the previous frame before drawing the current one.
        window->clear(sf::Color::Black);

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
                float windowWidth = windowManager.WorldToWindow(spr.bounds.width);
                float windowHeight = windowManager.WorldToWindow(spr.bounds.height);
                sf::Sprite& sprite = spriteManager.GetSprite(spr.textureID);
                float scaleX = windowWidth/sprite.getTextureRect().size.x;
                float scaleY = windowHeight/sprite.getTextureRect().size.y;

                position = windowManager.WorldToWindow(loc.loc + spr.bounds.topLeft);
                sprite.setScale({scaleX,scaleY});
            }

            // Each entity may share a sprite ID, so update its position
            // immediately before drawing it.
            sf::Sprite& sprite = spriteManager.GetSprite(spr.textureID);
            sprite.setPosition({position.x, position.y});
            

            window->draw(sprite);
        }

        //temporary for debug of collision objects:
        for (const Entity& entity : em) {
            EntityID id = entity.GetID();
            if (!em.validEntity(id)) {
                continue;
            }
            if(em.template HasComponent<LocationComponent>(id) && em.template HasComponent<CollisionComponent>(id)){
                // World-space sprites need a location component.
                auto& loc = em.template GetComponent<LocationComponent>(id);
                auto& coll = em.template GetComponent<CollisionComponent>(id);
                Rect collLocRect = GetWorldBox(loc,coll);
                Rect convCollLocRect = windowManager.WorldToWindow(collLocRect);
                windowManager.DebugFrameRect(convCollLocRect,255,0,0);   

            }
        }

        window->display();

        return kContinue;
    }
};

}  // namespace ECSEngine
