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
                break;
            }
        }
        return kContinue; 
    }
};

}  // namespace ECSEngine
