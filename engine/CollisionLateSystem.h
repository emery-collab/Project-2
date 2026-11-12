#pragma once

#include "CollisionComponent.h"
#include "EntityManager.h"
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

            if (!em.validEntity(id)) {
                continue;
            }

            if (!em.template HasComponent<CollisionComponent>(id)) {
                continue;
            }

            auto& collision = em.template GetComponent<CollisionComponent>(id);

            collision.previousCollisionBox = collision.collisionBox;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
