#pragma once

#include "CollisionComponent.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class CollisionLateSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionLateSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id) ||
                !em.template HasComponent<CollisionComponent>(id) ||
                !em.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            auto& collision = em.template GetComponent<CollisionComponent>(id);
            const auto& location = em.template GetComponent<LocationComponent>(id);

            // Save the collision box's world position before movement.
            collision.previousCollisionBox = Rect(
                Point2D(location.loc.x + collision.collisionBox.topLeft.x,
                        location.loc.y + collision.collisionBox.topLeft.y),
                collision.collisionBox.width,
                collision.collisionBox.height);
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
