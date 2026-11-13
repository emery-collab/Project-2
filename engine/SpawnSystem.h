#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "CollisionComponent.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();

        // Store spawn information so entities can be created after iteration.
        struct SpawnData {Point2D location; SpriteID textureID; std::string spawnType;};

        std::vector<SpawnData> pendingSpawns;

        // Random number generator used to give each star a random direction.
        static std::mt19937 generator(std::random_device{}());
        static std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * 3.14159265f);

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id) || !em.template HasComponent<SpawnComponent>(id) || !em.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            auto& spawn = em.template GetComponent<SpawnComponent>(id);

            // Stop spawning when the requested number has been created.
            if (spawn.totalSpawns <= 0) {
                continue;
            }

            // Count down once per frame.
            --spawn.timeToNextSpawn;

            if (spawn.timeToNextSpawn <= 0) {
                const auto& location = em.template GetComponent<LocationComponent>(id);

                pendingSpawns.push_back({location.loc, spawn.spawnTextureID, spawn.spawnType});
                --spawn.totalSpawns;

                // Reset the timer for the next spawn.
                spawn.timeToNextSpawn = std::max(1, spawn.timeBetweenSpawns);
            }
        }

        // Create entities only after the entity iteration has finished.
        for (const SpawnData& data : pendingSpawns) {
            EntityID starID = em.CreateEntity(data.spawnType);

            // Location component.
            LocationComponent location{};
            location.loc = data.location;
            em.AddComponent(starID, location);

            // Sprite component.
            SpriteComponent sprite{};
            sprite.textureID = data.textureID;
            sprite.bounds = Rect(Point2D(0.0f, 0.0f), 64, 64);
            sprite.worldSpace = true;
            em.AddComponent(starID, sprite);

            // Collision component.
            CollisionComponent collision{};
            collision.staticEntity = false;
            collision.doesDamage = false;
            collision.onGround = false;
            collision.onCeiling = false;
            collision.onLeftWall = false;
            collision.onRightWall = false;
            collision.collisionBox = Rect(Point2D(0.0f, 0.0f), 64, 64);
            collision.previousCollisionBox = Rect(data.location, 64, 64);
            em.AddComponent(starID, collision);

            // Movement component with a random direction.
            float angle = angleDistribution(generator);
            float speed = 250.0f;

            MovementComponent movement{};
            movement.velocity = Point2D(std::cos(angle) * speed, std::sin(angle) * speed);
            em.AddComponent(starID, movement);
        }

        return kContinue;
    }
};

}  // namespace ECSEngine