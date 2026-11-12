#pragma once

#include <vector>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;

    // Returns the time elapsed since the previous frame, in seconds.
    virtual float GetDeltaTime() const = 0;
};

template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

    float GetDeltaTime() const override { return mDeltaTime; }

private:
    bool ProcessEvents();
    void DebugDraw();

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // Tracks elapsed time between frames.
    sf::Clock mClock;
    float mDeltaTime = 0.0f;

    // For debugging - pause and step the engine
    bool running, step;
    int debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name), running(true), step(false), debug(0) {}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {}

template <typename... Components>
void ECSEngine<Components...>::Run() {

    auto window = mWindowManager.GetWindow();
    while (window->isOpen() && running) {

        // 1. Calculate elapsed time since the previous frame.
        mDeltaTime = mClock.restart().asSeconds();

        // 2. Process window and keyboard events.
        if (!ProcessEvents()) {
            break;
        }

        // 3. Clear the previous frame.
        window->clear();

        // 4. Run all registered systems.
        if (!mSystemManager.Run(this)) {
            break;
        }
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    auto window = mWindowManager.GetWindow();
    auto& em = mEntityManager;

    // Save the previous frame's keyboard state.
    for (const Entity& entity : em) {
        EntityID id = entity.GetID();

        if (!em.validEntity(id)) {
            continue;
        }

        if (!em.template HasComponent<InputComponent>(id)) {
            continue;
        }

        auto& input = em.template GetComponent<InputComponent>(id);

        input.lastKeydown = input.keydown;
    }

    // Process all pending SFML events.
    while (const std::optional event = window->pollEvent()) {

        // Handle window close.
        if (event->is<sf::Event::Closed>()) {
            window->close();
        }

        // Handle a key press.
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {

            std::size_t key = static_cast<std::size_t>(keyPressed->scancode);

            for (const Entity& entity : em) {
                EntityID id = entity.GetID();

                if (!em.validEntity(id)) {
                    continue;
                }

                if (!em.template HasComponent<InputComponent>(id)) {
                    continue;
                }

                auto& input = em.template GetComponent<InputComponent>(id);

                if (key < input.keydown.size()) {
                    input.keydown.set(key);
                }
            }
        }

        // Handle a key release.
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            
            std::size_t key = static_cast<std::size_t>(keyReleased->scancode);

            for (const Entity& entity : em) {
                EntityID id = entity.GetID();

                if (!em.validEntity(id)) {
                    continue;
                }

                if (!em.template HasComponent<InputComponent>(id)) {
                    continue;
                }

                auto& input = em.template GetComponent<InputComponent>(id);

                if (key < input.keydown.size()) {
                    input.keydown.reset(key);
                }
            }
        }
    }

    return window->isOpen();
}

template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {}

}  // namespace ECSEngine
