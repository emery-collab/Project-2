#pragma once

#include <unordered_map>

#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
#include "CameraComponent.h"
#include "WindowManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& em = engine->GetEntityManager();
        auto& window = engine->GetWindowManager();
        for (const Entity& entity : em) {
            const EntityID cameraID = entity.GetID();
            if (em.validEntity(cameraID) && em.template HasComponent<CameraComponent>(cameraID)) {
                auto& cam = em.template GetComponent<CameraComponent>(cameraID);
                window.SetWorldCenter(cam.position);
                window.SetWorldScale(cam.worldUnitsPerPixel);
                auto& follower = em.template GetComponent<CameraFollower>(cameraID);
                auto& playerloc = em.template GetComponent<LocationComponent>(follower.entityID);
                Point2D playerPoint2d = window.WorldToWindow(playerloc.loc);
                float playerX = playerPoint2d.x+50;
                float leftEdgeH = cam.hardExclusion.topLeft.x;
                float rightEdgeH = cam.hardExclusion.topLeft.x + cam.hardExclusion.width;
                float leftEdgeS = cam.softExclusion.topLeft.x;
                float rightEdgeS = cam.softExclusion.topLeft.x + cam.softExclusion.width;
                
                if(playerX < leftEdgeH){
                    float difference = cam.worldUnitsPerPixel*(leftEdgeH - playerX);
                    cam.position.x = cam.position.x - difference;
                    //std::cout << "hard left: " << playerPoint2d.x << "\n";
                }
                else if(playerX > rightEdgeH){
                    //std::cout << "hard right: " << playerPoint2d.x << "\n";
                    float difference = cam.worldUnitsPerPixel*(playerX - rightEdgeH);
                    cam.position.x = cam.position.x + difference;
                }
                else if(playerX >= leftEdgeH && playerX <= leftEdgeS ){
                    float difference = cam.worldUnitsPerPixel*(leftEdgeS - playerX);
                    float dt = engine->GetDeltaTime();
                    float step = difference * cam.speed * dt;
                    //std::cout << "soft left: " << playerPoint2d.x << "\n";
                    if(step < difference)
                        cam.position.x = cam.position.x - step;
                    else{
                        cam.position.x = cam.position.x - difference;
                    }
                }
                else if(playerX <= rightEdgeH && playerX >= rightEdgeS ){
                    //std::cout << "soft right: " << playerPoint2d.x << "\n";
                    float difference = cam.worldUnitsPerPixel * (playerX - rightEdgeS);
                    float dt = engine->GetDeltaTime();
                    float step = difference * cam.speed * dt;
                    if(step < difference)
                        cam.position.x = cam.position.x + step;
                    else{
                        cam.position.x = cam.position.x + difference;
                    }
                }
                // Apply a temporary shake without modifying the camera's actual position.
                auto& shake = em.template GetComponent<CameraShake>(cameraID);
                
                Point2D shakeOffset(0.0f, 0.0f);

                if (shake.frameLimit > 0) {
                    // Gradually reduce shake strength as the animation progresses.
                    float progress = static_cast<float>(shake.frameLimit) / 18.0f;
                    float strength = progress * progress;

                    // Alternate the direction while fading out.
                    float direction = (shake.frameLimit % 2 == 0) ? 1.0f : -1.0f;
                    shakeOffset = shake.offset * (strength * direction);
                    shake.frameLimit--;
                }

                window.SetWorldCenter(cam.position + shakeOffset);
                
                break;
            }

        }
        return kContinue; 
    }
};

}  // namespace ECSEngine
