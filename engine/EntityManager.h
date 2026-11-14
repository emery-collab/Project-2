#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <tuple>
#include <string>

#include "Component.h"
#include "Entity.h"
#include "pack.h"

namespace ECSEngine {

// Owns all entities and components. EntityID = index into mEntities.
template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();

    // Reuses IDs of removed entities, so an old ID may become a new entity.
    [[nodiscard]] EntityID CreateEntity(const std::string& name)  {
        EntityID id;
        if (!mFreeEntities.empty()) {
            id = mFreeEntities.back();
            mFreeEntities.pop_back();
            mEntities[id] = Entity(id,name);
            mEntityValid[id] = true;
        } else {
            id = mComponentIndices.size();
            mComponentIndices.emplace_back();
            mEntities.emplace_back(Entity(id,name));
            mEntityValid.push_back(true);
        }

        mComponentIndices[id].fill(kNullComponent);
        return id;
    }
    // Use to skip removed entities while iterating.
    bool validEntity(EntityID id) const {
        return id < mEntityValid.size() && mEntityValid[id];
    }

    // Removes the entity and its components; its ID can be reused.
    void RemoveEntity(EntityID entity) {
        assert(validEntity(entity) && "Invalid Entity");
        // AI-written (Claude): removes each component type the entity has.
        ([&] { if (HasComponent<Components>(entity)) RemoveComponent<Components>(entity); }(), ...);
        mEntityValid[entity] = false;
        mFreeEntities.push_back(entity);
    }

    // Copies component in. May move T's storage, invalidating old GetComponent<T> references.
    template <typename T>
    void AddComponent(EntityID entity, T component) {
        assert(validEntity(entity) && "Invalid Entity");
        assert(!HasComponent<T>(entity) && "Entity Does not have specified type");
        size_t column = slot<T>;

        ComponentStorage<T>& cs = GetComponentStorage<T>();
        size_t index;
        if (!cs.mFreeList.empty()) {
            std::size_t i = cs.mFreeList.back();
            cs.mFreeList.pop_back();
            cs.mStorage[i] = component;
            cs.mValid[i] = true;
            index = i;
        }else{
            cs.mStorage.push_back(component);
            cs.mValid.push_back(true);
            index = cs.mStorage.size() -1;
        }

        mComponentIndices[entity][column] = index;
    }

    // Entity must have a T (not checked). Its slot is reused later.
    template <typename T>
    void RemoveComponent(EntityID entity) {
        static_assert(slot<T> != -1, "T is not in this EntityManager");
        size_t column = slot<T>;
        size_t slot = mComponentIndices[entity][column];
        ComponentStorage<T>& storage = GetComponentStorage<T>();
        storage.mValid[slot] = false;
        storage.mFreeList.push_back(slot);
        mComponentIndices[entity][column] = kNullComponent;
    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        if (!validEntity(entity)) {
            return false;
        }
        
        static_assert(slot<T> != -1, "T is not in this EntityManager");
        size_t column = slot<T>;
        size_t registryIndex = mComponentIndices[entity][column];
        return registryIndex != kNullComponent;
    }

    // Check HasComponent<T> first. Reference is valid until the next AddComponent<T> or removal.
    template <typename T>
    T& GetComponent(EntityID entity) {
        static_assert(slot<T> != -1, "T is not in this EntityManager");        
        size_t column = slot<T>;
        //std::cout<< "Column: " << column << "\n";
        size_t storageIndex = mComponentIndices[entity][column];
        //std::cout<< "storage index: " << storageIndex << "\n";
        ComponentStorage<T>& storage = GetComponentStorage<T>();
        return storage.mStorage[storageIndex];
    }

    // Lives as long as the manager. Only use slots where mValid is true.
    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mComponents);
    }
    
    // Includes removed entities: skip ones where validEntity is false.
    using tEntity = std::vector<Entity>;
    using const_iterator = tEntity::const_iterator;
    const_iterator cbegin() const { return mEntities.begin(); }
    const_iterator cend() const { return mEntities.end(); }
    const_iterator begin() { return mEntities.begin(); }
    const_iterator end() { return mEntities.end(); }

    private:
    std::vector<Entity> mEntities;
    std::vector<size_t> mFreeEntities;
    std::vector<bool> mEntityValid;
    static constexpr std::size_t NUM_COMPONENTS = sizeof...(Components);
    std::vector<std::array<std::size_t, NUM_COMPONENTS>> mComponentIndices;
    std::tuple<ComponentStorage<Components>...> mComponents;
    using tPack = Pack<Components...>;

    template <typename T>
    static constexpr int slot = tPack::template index<T>;

};
}  // namespace ECSEngine
