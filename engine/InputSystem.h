
#pragma once

#include <algorithm>

#include "Keyboard.h"

namespace ECSEngine {

const float kMaxRunVelocity = 400.0f;
const float kDashVelocity = 1300.0f;
const float kJumpVelocity = 550.0f;
const float kGroundRunAcceleration = 2000.0f;
const float kAirRunAcceleration = 1200.0f;
const float kDashInterval = 1.0f;
constexpr float kWallJumpHorizontalVelocity = 200.0f;

// Gradually move a value toward its target.
inline float Approach(float current, float target, float amount) {
    if (current < target) {
        return std::min(current + amount, target);
    }
    return std::max(current - amount, target);
}

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();

        const float dt = std::clamp(engine->GetDeltaTime(), 0.0f, 0.05f);

        for (const Entity& entity : em) {
            EntityID id = entity.GetID();

            if (!em.validEntity(id)) {
                continue;
            }

            if (!em.template HasComponent<InputComponent>(id) ||
                !em.template HasComponent<MovementComponent>(id) ||
                !em.template HasComponent<CollisionComponent>(id)) {
                continue;
            }

            auto& input = em.template GetComponent<InputComponent>(id);
            auto& movement = em.template GetComponent<MovementComponent>(id);
            auto& collision = em.template GetComponent<CollisionComponent>(id);

            // Current keyboard state.
            const bool left = input.keydown[Key::Left];
            const bool right = input.keydown[Key::Right];
            const bool jump = input.keydown[Key::Z];
            const bool dash = input.keydown[Key::C];

            // Detect keys pressed during this frame.
            const bool jumpPressed = jump && !input.lastKeydown[Key::Z];
            const bool dashPressed = dash && !input.lastKeydown[Key::C];

            // Reduce dash cooldown in seconds.
            input.lastDash = std::max(0.0f, input.lastDash - dt);

            // Determine horizontal movement direction.
            float direction = 0.0f;

            if (left && !right) {
                direction = -1.0f;
            } else if (right && !left) {
                direction = 1.0f;
            }

            // Apply horizontal acceleration.
            const float acceleration =collision.onGround ? kGroundRunAcceleration : kAirRunAcceleration;
            const float targetVelocity = direction * kMaxRunVelocity;

            movement.velocity.x = Approach(movement.velocity.x, targetVelocity, acceleration * dt);

            // Normal jumping and wall jumping.
            if (jumpPressed) {
                if (collision.onGround) {
                    movement.velocity.y = -kJumpVelocity;
                    engine->GetSoundManager().GetSound("jump").play();
                } else {
                    // Wall jumping requires falling and pushing into a wall.
                    const bool pushingLeft = collision.onLeftWall && left;
                    const bool pushingRight = collision.onRightWall && right;

                    if (movement.velocity.y > 0.0f &&
                        (pushingLeft || pushingRight)) {

                        movement.velocity.y = -kJumpVelocity;

                        // Launch away from the wall.
                        movement.velocity.x = pushingLeft ? kWallJumpHorizontalVelocity : -kWallJumpHorizontalVelocity;

                        engine->GetSoundManager().GetSound("jump").play();
                    }
                }
            }

            // Variable jump height: releasing Z cuts upward velocity.
            if (!jump && movement.velocity.y < 0.0f) {
                movement.velocity.y = std::max(movement.velocity.y, -kJumpVelocity * 0.35f);
            }

            // Horizontal dash, usable once per second.
            if (dashPressed && input.lastDash <= 0.0f) {
                float dashDirection = direction;

                if (dashDirection == 0.0f) {
                    dashDirection = movement.velocity.x < 0.0f ? -1.0f : 1.0f;
                }

                movement.velocity.x = dashDirection * kDashVelocity;
                input.lastDash = kDashInterval;
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
