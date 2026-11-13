#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class CollisionSystem : public System<Components...> {
private:
    static Rect GetWorldBox(const LocationComponent& location,
                            const CollisionComponent& collision) {
        return Rect(
            Point2D(location.loc.x + collision.collisionBox.topLeft.x,
                    location.loc.y + collision.collisionBox.topLeft.y),
            collision.collisionBox.width,
            collision.collisionBox.height);
    }

    static bool Overlaps(const Rect& a, const Rect& b) {
        return a.topLeft.x < b.topLeft.x + b.width &&
               a.topLeft.x + a.width > b.topLeft.x &&
               a.topLeft.y < b.topLeft.y + b.height &&
               a.topLeft.y + a.height > b.topLeft.y;
    }

    static bool IsStar(const Entity& entity) {
        return entity.GetType() == "star";
    }

    // helper function for camera shake 
    template <typename EngineType>
    static void StartCameraShake(EngineType* engine,
                                const Point2D& direction,
                                float impactSpeed) {
        auto& em = engine->GetEntityManager();

        // Scale the shake according to the impact speed.
        float magnitude = std::clamp(impactSpeed * 0.015f, 1.0f, 7.0f);

        for (const Entity& entity : em) {
            EntityID cameraID = entity.GetID();

            if (!em.validEntity(cameraID) || !em.template HasComponent<CameraShake>(cameraID)) {
                continue;
            }

            auto& shake = em.template GetComponent<CameraShake>(cameraID);

            shake.frameLimit = 18;
            shake.offset = direction * magnitude;
            break;
        }
    }

public:
    const std::string& GetName() const override {
        static std::string name("CollisionSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();
        constexpr float kEpsilon = 0.001f;

        // Resolve every moving, non-static entity against static collision
        // geometry. The previous box is in world coordinates (saved by
        // CollisionLateSystem before MovementSystem runs).
        for (const Entity& entity : em) {
            const EntityID id = entity.GetID();

            if (!em.validEntity(id) ||
                !em.template HasComponent<CollisionComponent>(id) ||
                !em.template HasComponent<LocationComponent>(id) ||
                !em.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            auto& collision = em.template GetComponent<CollisionComponent>(id);
            if (collision.staticEntity) {
                continue;
            }

            // Remember whether the entity was grounded last frame.
            const bool wasOnGround = collision.onGround;

            // Reset contact flags after InputSystem and GravitySystem
            // have used the previous frame's collision information.
            collision.onGround = false;
            collision.onCeiling = false;
            collision.onLeftWall = false;
            collision.onRightWall = false;

            auto& location = em.template GetComponent<LocationComponent>(id);
            auto& movement = em.template GetComponent<MovementComponent>(id);

            for (const Entity& obstacle : em) {
                const EntityID obstacleID = obstacle.GetID();

                if (obstacleID == id || !em.validEntity(obstacleID) ||
                    !em.template HasComponent<CollisionComponent>(obstacleID) ||
                    !em.template HasComponent<LocationComponent>(obstacleID)) {
                    continue;
                }

                const auto& obstacleCollision =
                    em.template GetComponent<CollisionComponent>(obstacleID);
                if (!obstacleCollision.staticEntity) {
                    continue;
                }
                if (obstacleCollision.doesDamage && entity.GetType() == "player" ) {
                    continue;
                }

                const auto& obstacleLocation =
                    em.template GetComponent<LocationComponent>(obstacleID);
                const Rect obstacleBox = GetWorldBox(obstacleLocation, obstacleCollision);
                Rect currentBox = GetWorldBox(location, collision);

                if (!Overlaps(currentBox, obstacleBox)) {
                    continue;
                }
                

                const Rect& previousBox = collision.previousCollisionBox;
                const bool star = IsStar(entity);

                // Prefer the axis and direction indicated by the previous
                // frame's box. This avoids resolving a landing as a side hit.
                const bool crossedTop = previousBox.topLeft.y + previousBox.height <= obstacleBox.topLeft.y + kEpsilon;
                const bool crossedBottom = previousBox.topLeft.y >= obstacleBox.topLeft.y + obstacleBox.height - kEpsilon;
                const bool crossedLeft = previousBox.topLeft.x + previousBox.width <= obstacleBox.topLeft.x + kEpsilon;
                const bool crossedRight = previousBox.topLeft.x >= obstacleBox.topLeft.x + obstacleBox.width - kEpsilon;

                // Save the player's position and velocity before collision resolution for screen shake purposes
                const Point2D positionBeforeResolution = location.loc;
                const Point2D velocityBeforeResolution = movement.velocity;

                if (crossedTop || crossedBottom) {
                    if (crossedTop || (!crossedBottom &&
                        std::abs(movement.velocity.y) > 0.0f)) {
                        // Land on the top of the obstacle.
                        location.loc.y = obstacleBox.topLeft.y - collision.collisionBox.topLeft.y - collision.collisionBox.height;
                        if (star) {
                            movement.velocity.y = -std::abs(movement.velocity.y) * 0.8f;
                        } else {
                            movement.velocity.y = 0.0f;
                            collision.onGround = true;
                        }
                    } else {
                        // Hit the underside of the obstacle.
                        location.loc.y = obstacleBox.topLeft.y + obstacleBox.height - collision.collisionBox.topLeft.y;
                        if (star) {
                            movement.velocity.y = std::abs(movement.velocity.y) * 0.8f;
                        } else {
                            movement.velocity.y = 0.0f;
                            collision.onCeiling = true;
                        }
                    }
                } else if (crossedLeft || crossedRight) {
                    if (crossedLeft || (!crossedRight &&
                        std::abs(movement.velocity.x) > 0.0f)) {
                        // Hit the obstacle's left side.
                        location.loc.x = obstacleBox.topLeft.x - collision.collisionBox.topLeft.x - collision.collisionBox.width;
                        if (star) {
                            movement.velocity.x = -std::abs(movement.velocity.x) * 0.8f;
                        } else {
                            movement.velocity.x = 0.0f;
                            collision.onRightWall = true;
                        }
                    } else {
                        // Hit the obstacle's right side.
                        location.loc.x = obstacleBox.topLeft.x + obstacleBox.width - collision.collisionBox.topLeft.x;
                        if (star) {
                            movement.velocity.x = std::abs(movement.velocity.x) * 0.8f;
                        } else {
                            movement.velocity.x = 0.0f;
                            collision.onLeftWall = true;
                        }
                    }
                } else {
                    // Fallback for an initial overlap or a large movement
                    // that crossed more than one boundary in a single frame.
                    const float overlapLeft = currentBox.topLeft.x + currentBox.width - obstacleBox.topLeft.x;
                    const float overlapRight = obstacleBox.topLeft.x + obstacleBox.width - currentBox.topLeft.x;
                    const float overlapTop = currentBox.topLeft.y + currentBox.height - obstacleBox.topLeft.y;
                    const float overlapBottom = obstacleBox.topLeft.y + obstacleBox.height - currentBox.topLeft.y;
                    const float minX = std::min(overlapLeft, overlapRight);
                    const float minY = std::min(overlapTop, overlapBottom);

                    if (minY <= minX) {
                        if (overlapTop <= overlapBottom) {
                            location.loc.y -= overlapTop;
                            if (star) movement.velocity.y = -std::abs(movement.velocity.y) * 0.8f;
                            else {
                                movement.velocity.y = 0.0f;
                                collision.onGround = true;
                            }
                        } else {
                            location.loc.y += overlapBottom;
                            if (star) movement.velocity.y = std::abs(movement.velocity.y) * 0.8f;
                            else {
                                movement.velocity.y = 0.0f;
                                collision.onCeiling = true;
                            }
                        }
                    } else {
                        if (overlapLeft <= overlapRight) {
                            location.loc.x -= overlapLeft;
                            if (star) movement.velocity.x = -std::abs(movement.velocity.x) * 0.8f;
                            else {
                                movement.velocity.x = 0.0f;
                                collision.onRightWall = true;
                            }
                        } else {
                            location.loc.x += overlapRight;
                            if (star) movement.velocity.x = std::abs(movement.velocity.x) * 0.8f;
                            else {
                                movement.velocity.x = 0.0f;
                                collision.onLeftWall = true;
                            }
                        }
                    }
                }
                // Screen shake for player collisions with static obstacles.
                if (entity.GetType() == "player") {
                    const float correctionX = location.loc.x - positionBeforeResolution.x;
                    const float correctionY = location.loc.y - positionBeforeResolution.y;

                    if (std::abs(correctionX) > 0.001f || std::abs(correctionY) > 0.001f) {

                        Point2D direction(0.0f, 0.0f);
                        float impactSpeed = 0.0f;

                        // Use the axis with the larger positional correction.
                        if (std::abs(correctionX) > std::abs(correctionY)) {
                            direction.x = correctionX < 0.0f ? 1.0f : -1.0f;
                            impactSpeed = std::abs(velocityBeforeResolution.x);
                        } else {
                            direction.y = correctionY < 0.0f ? 1.0f : -1.0f;
                            impactSpeed = std::abs(velocityBeforeResolution.y);
                        }

                        if (impactSpeed > 30.0f) {
                            StartCameraShake(engine, direction, impactSpeed);
                        }
                    }
                }
            // Play landing sound only when the player first touches the ground.
            if (entity.GetType() == "player" && !wasOnGround && collision.onGround) {
                engine->GetSoundManager().GetSound("grassFootsteps").play();
            }
            
            // end collision loop for object instance
            }
        }
        //Star vs star 
        std::vector<EntityID> starList;
        for (const Entity& star : em) {
            const EntityID starID = star.GetID();
            if (IsStar(star)){
                if(em.validEntity(starID)){
                    starList.push_back(starID);
                }

            }
        }
        for(size_t i = 0; i < starList.size();++i){
            for(size_t j = i+1; j < starList.size();++j){
                auto& aLoc = em.template GetComponent<LocationComponent>(starList[i]);
                auto& bLoc = em.template GetComponent<LocationComponent>(starList[j]);
                auto& aCol = em.template GetComponent<CollisionComponent>(starList[i]);
                auto& bCol = em.template GetComponent<CollisionComponent>(starList[j]);

                Rect aBox = GetWorldBox(aLoc,aCol);
                Rect bBox = GetWorldBox(bLoc,bCol);

                if(Overlaps(aBox, bBox)){
                    auto& aMov = em.template GetComponent<MovementComponent>(starList[i]);
                    auto& bMov = em.template GetComponent<MovementComponent>(starList[j]);
                    float diffX = bLoc.loc.x - aLoc.loc.x;
                    float diffY = bLoc.loc.y - aLoc.loc.y;
                    float diffVelocityX = bMov.velocity.x - aMov.velocity.x;
                    float diffVelocityY = bMov.velocity.y - aMov.velocity.y;
                    //dot product
                    float dot = diffX*diffVelocityX + diffY*diffVelocityY;
                    if(dot < 0){
                        bMov.velocity.x *= -0.95;
                        aMov.velocity.x *= -0.95;
                        bMov.velocity.y *= -0.95;
                        aMov.velocity.y *= -0.95;
                    }
                    
                }
                
            }
        }

        // Handle player interactions after static collision resolution.
        EntityID playerID = 0;
        bool foundPlayer = false;
        for (const Entity& entity : em) {
            if (em.validEntity(entity.GetID()) && entity.GetType() == "player") {
                playerID = entity.GetID();
                foundPlayer = true;
                break;
            }
        }

        if (foundPlayer &&
            em.template HasComponent<LocationComponent>(playerID) &&
            em.template HasComponent<CollisionComponent>(playerID)) {
            auto& playerLocation = em.template GetComponent<LocationComponent>(playerID);
            auto& playerCollision = em.template GetComponent<CollisionComponent>(playerID);
            const Rect playerBox = GetWorldBox(playerLocation, playerCollision);

            std::vector<EntityID> collectedStars;
            bool playerDamaged = false;

            for (const Entity& entity : em) {
                const EntityID otherID = entity.GetID();
                if (otherID == playerID || !em.validEntity(otherID) ||
                    !em.template HasComponent<LocationComponent>(otherID) ||
                    !em.template HasComponent<CollisionComponent>(otherID)) {
                    continue;
                }

                auto& otherCollision = em.template GetComponent<CollisionComponent>(otherID);
                const auto& otherLocation = em.template GetComponent<LocationComponent>(otherID);
                if (!Overlaps(playerBox, GetWorldBox(otherLocation, otherCollision))) {
                    continue;
                }

                if (otherCollision.doesDamage) {
                    playerDamaged = true;
                } else if (!otherCollision.staticEntity && IsStar(entity)) {
                    collectedStars.push_back(otherID);
                }
            }

            if (!collectedStars.empty()) {
                for (const Entity& entity : em) {
                    const EntityID scoreID = entity.GetID();
                    if (em.validEntity(scoreID) && entity.GetType() == "score" &&
                        em.template HasComponent<ScoreComponent>(scoreID)) {
                        auto& score = em.template GetComponent<ScoreComponent>(scoreID);
                        score.score += static_cast<int>(collectedStars.size());
                        break;
                    }
                }

                // Remove after iteration so EntityManager's entity list is
                // not modified while it is being traversed.
                for (EntityID starID : collectedStars) {
                    if (em.validEntity(starID)) {
                        em.RemoveEntity(starID);
                    }
                }

                engine->GetSoundManager().GetSound("gem").play();
            }

            if (playerDamaged) {
                for (const Entity& entity : em) {
                    const EntityID respawnID = entity.GetID();
                    if (em.validEntity(respawnID) &&
                        entity.GetType() == "player" &&
                        em.template HasComponent<RespawnComponent>(respawnID)) {
                        const auto& respawn = em.template GetComponent<RespawnComponent>(respawnID);
                        playerLocation.loc = respawn.loc;

                        if (em.template HasComponent<MovementComponent>(playerID)) {
                            auto& velocity = em.template GetComponent<MovementComponent>(playerID);
                            velocity.velocity = Point2D(0.0f, 0.0f);
                        }
                        break;
                    }
                }

                for (const Entity& entity : em) {
                    const EntityID cameraID = entity.GetID();
                    if (em.validEntity(cameraID) &&
                        em.template HasComponent<CameraShake>(cameraID)) {
                        auto& shake = em.template GetComponent<CameraShake>(cameraID);
                        shake.frameLimit = 10;
                        break;
                    }
                }
                engine->GetSoundManager().GetSound("error").play();
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine