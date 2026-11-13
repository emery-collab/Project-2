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
                float leftEdgeH = cam.hardExclusion.topLeft.x;
                float rightEdgeH = cam.hardExclusion.topLeft.x + cam.hardExclusion.width;
                float leftEdgeS = cam.softExclusion.topLeft.x;
                float rightEdgeS = cam.softExclusion.topLeft.x + cam.softExclusion.width;
                
                if(playerPoint2d.x < leftEdgeH){
                    float difference = cam.worldUnitsPerPixel*(leftEdgeH - playerPoint2d.x);
                    cam.position.x = cam.position.x - difference;
                    //std::cout << "hard left: " << playerPoint2d.x << "\n";
                }
                else if(playerPoint2d.x > rightEdgeH){
                    //std::cout << "hard right: " << playerPoint2d.x << "\n";
                    float difference = cam.worldUnitsPerPixel*(playerPoint2d.x - rightEdgeH);
                    cam.position.x = cam.position.x + difference;
                }
                else if(playerPoint2d.x >= leftEdgeH && playerPoint2d.x <= leftEdgeS ){
                    float difference = cam.worldUnitsPerPixel*(leftEdgeS - playerPoint2d.x);
                    float dt = engine->GetDeltaTime();
                    float step = difference * cam.speed * dt;
                    //std::cout << "soft left: " << playerPoint2d.x << "\n";
                    if(step < difference)
                        cam.position.x = cam.position.x - step;
                    else{
                        cam.position.x = cam.position.x - difference;
                    }
                }
                else if(playerPoint2d.x <= rightEdgeH && playerPoint2d.x >= rightEdgeS ){
                    //std::cout << "soft right: " << playerPoint2d.x << "\n";
                    float difference = cam.worldUnitsPerPixel * (playerPoint2d.x - rightEdgeS);
                    float dt = engine->GetDeltaTime();
                    float step = difference * cam.speed * dt;
                    if(step < difference)
                        cam.position.x = cam.position.x + step;
                    else{
                        cam.position.x = cam.position.x + difference;
                    }
                }
                auto& shake = em.template GetComponent<CameraShake>(cameraID);
                if(shake.frameLimit>0){
                    float currShake = shake.frameLimit / 10.0f;
                    if(shake.frameLimit % 2 ==0){
                        cam.position.y += currShake * shake.offset.y;
                        window.SetWorldCenter(cam.position);
                    }
                    else{
                        cam.position.y -= currShake * shake.offset.y;
                        window.SetWorldCenter(cam.position);
                    }
                    shake.frameLimit--;

                }
                break;
            }

        }
        return kContinue; 
    }
};

}  // namespace ECSEngine
