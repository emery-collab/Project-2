#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <type_traits>

#include "Component.h"
#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();
    [[nodiscard]] EntityID CreateEntity(const std::string& name)  {
        EntityID id;
        if (!mFreeEntities.empty()) {
            id = mFreeEntities.back();
            mFreeEntities.pop_back();
            mEntityValid[id] = true;
        } else {
            id = mComponentIndices.size();
            mComponentIndices.emplace_back();
            mEntityValid.push_back(true);
        }

        mComponentIndices[id].fill(kNullComponent);
        Entity entity = Entity(id,name);
        tEntity.push_back(entity);
        
        return id;
    }
    bool validEntity(EntityID id) const {
        return id < mEntityValid.size() && mEntityValid[id];
    }

    void RemoveEntity(EntityID entity) {
        if (!validEntity(entity.mID)) {
            return;
        }
        tEntity[entity.mID] = T{};
        mFreeEntities[entity.mID] = false;
        mFreeEntities.push_back(entity.mID);
    }

    template <typename T>
    void AddComponent(EntityID entity, T component) {
        assert(validEntity(entity.mID) && "Invalid Entity");
        assert(!HasComponent<T>(entity) && "Entity Does not have specified type");
        size_t column = indexOf<T>();
        ComponentStorage<T>& = 
    }

    template <typename T>
    void RemoveComponent(EntityID entity) {

    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        if (!validEntity(entity.mId)) {
            return false;
        }
        size_t column = indexOf<Components>();
        size_t registryIndex = mComponentIndices[entity.mId][column];
        return registryIndex != kNullComponent;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        // Re-write code
        assert(hasComponent<T>(entity.mID) && "No component type here.");
        size_t column = indexOf<T>();
        size_t registryIndex = mComponentIndices[entity.mID][column];
        return dummy;
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        // Re-write code
        static ComponentStorage<T> dummy;
        return dummy;
    }
    
    using tEntity = std::vector<Entity>;
    using const_iterator = tEntity::const_iterator;
    const_iterator cbegin() const { return mEntities.begin(); }
    const_iterator cend() const { return mEntities.end(); }
    const_iterator begin() { return mEntities.begin(); }
    const_iterator end() { return mEntities.end(); }

    private:
    std::vector<EntityID> mFreeEntities;
    std::vector<bool> mEntityValid;
    std::vector<std::array<std::size_t, NUM_COMPONENTS>> mComponentIndices;
    std::vector<T> mStorage;
};

using EntityID = size_t;
class Entity {
public:
	EntityID GetID() const { return mID; }
private:
	Entity() = default;
	Entity(EntityID eid, const std::string &name)
	      :mID(eid), mType(name) {}
	EntityID mID;
	std::string mType;
	
	template <typename... T>
	friend class EntityManager;
};

}  // namespace ECSEngine
