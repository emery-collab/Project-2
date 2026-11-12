#pragma once

#include <algorithm>

#include "Keyboard.h"
#include "LocationComponent.h"
#include "CollisionComponent.h"
#include "InputComponent.h"
#include "EntityManager.h"
#include "SystemManager.h"

namespace ECSEngine
{

constexpr float kWallSlideVelocity = 100.0f;
constexpr float kFallingGravityMultiplier = 1.5f;
constexpr float kFastFallMultiplier = 2.0f;
constexpr float kMaximumFallVelocity = 900.0f;


template <typename... Components>
class GravitySystem : public System<Components...> {
public:
	const std::string & GetName() const override
	{ static std::string name("GravitySystem"); return name; }
	
	tSystemReturn Run(EngineView<Components...> *engine) override
	{
		auto& em = engine->GetEntityManager();
        const float dt = std::clamp(engine->GetDeltaTime(), 0.0f, 0.05f);

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id)) {
                continue;
            }

            if (!em.template HasComponent<GravityComponent>(id) || !em.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            auto& gravity = em.template GetComponent<GravityComponent>(id);

            auto& movement = em.template GetComponent<MovementComponent>(id);

            // Respect gravity disabled timer.
            if (gravity.disabledTimer > 0.0f) {
                gravity.disabledTimer = std::max(0.0f, gravity.disabledTimer - dt);
                continue;
            }

            bool fastFall = false;
            bool wallSliding = false;

            // Input and collision information are optional for
            // non-player entities.
            if (em.template HasComponent<InputComponent>(id) &&
                em.template HasComponent<CollisionComponent>(id)) {

                auto& input = em.template GetComponent<InputComponent>(id);
                auto& collision = em.template GetComponent<CollisionComponent>(id);

                fastFall = input.keydown[Key::Down];

                // Player must be falling and actively pushing
                // against the wall to wall slide.
                wallSliding =
                    movement.velocity.y > 0.0f &&
                    ((collision.onLeftWall &&
                      input.keydown[Key::Left]) ||
                     (collision.onRightWall &&
                      input.keydown[Key::Right]));
            }

            // Wall sliding limits downward velocity.
            if (wallSliding) {
                movement.velocity.y = std::min(movement.velocity.y, kWallSlideVelocity);
                continue;
            }

            // Determine gravity multiplier.
            float multiplier = 1.0f;

            // Falling is faster than rising.
            if (movement.velocity.y > 0.0f) {
                multiplier *= kFallingGravityMultiplier;
            }

            // Holding Down increases falling speed.
            if (fastFall && movement.velocity.y > 0.0f) {
                multiplier *= kFastFallMultiplier;
            }

            // Apply acceleration.
            movement.velocity = movement.velocity + gravity.acceleration * (dt * multiplier);

            // Prevent excessive falling speed.
            movement.velocity.y = std::min(movement.velocity.y, kMaximumFallVelocity);
        }

        return kContinue;
    }
	
};


}



