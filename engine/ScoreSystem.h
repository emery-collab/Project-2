#pragma once

#include <algorithm>

#include "EntityManager.h"
#include "LocationComponent.h"
#include "ScoreComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& em = engine->GetEntityManager();

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id) ||
                entity.GetType() != "score" ||
                !em.template HasComponent<ScoreComponent>(id)) {
                continue;
            }

            auto& score = em.template GetComponent<ScoreComponent>(id);

            // Keep the displayed score within the three available digits.
            int displayedScore = std::clamp(score.score, 0, 999);

            // Update each digit from right to left.
            for (int i = 2; i >= 0; --i) {
                int digit = displayedScore % 10;
                displayedScore /= 10;

                EntityID digitID = score.digitID[i];

                if (!em.validEntity(digitID) || !em.template HasComponent<SpriteComponent>(digitID)) {
                    continue;
                }

                auto& sprite = em.template GetComponent<SpriteComponent>(digitID);
                sprite.textureID = score.digitTextureID[digit];
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
