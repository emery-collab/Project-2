#pragma once

#include <algorithm>

#include "LocationComponent.h"
#include "EntityManager.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& em = engine->GetEntityManager();

        // Time elapsed since the previous frame.
        const float dt = std::clamp(engine->GetDeltaTime(), 0.0f, 0.05f);

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            // Skip entities that have been removed.
            if (!em.validEntity(id)) {
                continue;
            }

            // Movement requires both components.
            if (!em.template HasComponent<LocationComponent>(id) || !em.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            auto& location = em.template GetComponent<LocationComponent>(id);

            auto& movement = em.template GetComponent<MovementComponent>(id);

            // Update position using velocity and elapsed time.
            location.loc = location.loc + movement.velocity * dt;
        }

        return kContinue;
    }
    
};

}  // namespace ECSEngine
