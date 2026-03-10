#include "Config/pch.h"
#include "Physics/PhysicsEventCallback.h"

namespace physics {
    void PhysicsEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {
        if (pairHeader.flags & (PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1)) {
            CollisionPair p{ pairHeader.actors[0], pairHeader.actors[1] };
            m_activeCollisions.erase(p);
            m_cachedCollisionData.erase(p);
            return;
        }
        for (PxU32 i = 0; i < nbPairs; ++i) {
            const PxContactPair& cp = pairs[i];

            if (cp.flags & (PxContactPairFlag::eREMOVED_SHAPE_0 | PxContactPairFlag::eREMOVED_SHAPE_1)) { continue; }

            CollisionPair pair{ pairHeader.actors[0], pairHeader.actors[1] };

            PendingCollisionEvent e;
            BuildCollisionData(pairHeader, cp, e.collisionA, e.collisionB);

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                e.type = PendingCollisionEvent::Type::Enter;
                m_pendingCollisionEvents.push_back(e);
                m_activeCollisions.insert(pair);
                m_cachedCollisionData[pair] = { e.collisionA, e.collisionB };
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
                m_activeCollisions.insert(pair);
                m_cachedCollisionData[pair] = { e.collisionA, e.collisionB };
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
                e.type = PendingCollisionEvent::Type::Exit;
                m_pendingCollisionEvents.push_back(e);
                m_activeCollisions.erase(pair);
                m_cachedCollisionData.erase(pair);
            }
        }
    }

    void PhysicsEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count) { 
        for (PxU32 i = 0; i < count; ++i) {
            const PxTriggerPair& tp = pairs[i];

            TriggerPair pair{ tp.triggerActor, tp.otherActor };

            if (tp.flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) { 
                m_activeTriggers.erase(pair);
                continue; 
            }

            unsigned int entityA = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(tp.triggerActor->userData));
            unsigned int entityB = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(tp.otherActor->userData));

            if (tp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                m_pendingTriggerEvents.push_back({ entityA, entityB, PendingTriggerEvent::Type::Enter });
                m_activeTriggers.insert(pair);
            }
            else if (tp.status & PxPairFlag::eNOTIFY_TOUCH_LOST) {
                m_pendingTriggerEvents.push_back({ entityA, entityB, PendingTriggerEvent::Type::Exit });
                m_activeTriggers.erase(pair);
            }
        }
    }

    void PhysicsEventCallback::FlushEvents() {
        for (const auto& e : m_pendingCollisionEvents) {
            unsigned int idA = e.collisionA.thisEntityID;
            unsigned int idB = e.collisionB.thisEntityID;

            if (e.type == PendingCollisionEvent::Type::Enter) {
                if (CollisionEnterList.contains(idA)) { CollisionEnterList[idA].Invoke(e.collisionA); }
                if (CollisionEnterList.contains(idB)) { CollisionEnterList[idB].Invoke(e.collisionB); }
            }
            else {
                if (CollisionExitList.contains(idA)) { CollisionExitList[idA].Invoke(e.collisionA); }
                if (CollisionExitList.contains(idB)) { CollisionExitList[idB].Invoke(e.collisionB); }
            }
        }
        m_pendingCollisionEvents.clear();

        for (const auto& e : m_pendingTriggerEvents) {
            Collision triggerA{}, triggerB{};
            triggerA.thisEntityID = e.entityA;
            triggerA.otherEntityID = e.entityB;
            triggerB.thisEntityID = e.entityB;
            triggerB.otherEntityID = e.entityA;

            if (e.type == PendingTriggerEvent::Type::Enter) {
                if (TriggerEnterList.contains(e.entityA)) { TriggerEnterList[e.entityA].Invoke(triggerA); }
                if (TriggerEnterList.contains(e.entityB)) { TriggerEnterList[e.entityB].Invoke(triggerB); }
            }
            else {
                if (TriggerExitList.contains(e.entityA)) { TriggerExitList[e.entityA].Invoke(triggerA); }
                if (TriggerExitList.contains(e.entityB)) { TriggerExitList[e.entityB].Invoke(triggerB); }
            }
        }
        m_pendingTriggerEvents.clear();
    }

    void PhysicsEventCallback::ProcessCollisionStay() {
        for (const auto& pair : m_activeCollisions) {
            auto it = m_cachedCollisionData.find(pair);
            if (it == m_cachedCollisionData.end()) { continue; }

            const CachedCollisionData& cached = it->second;
            unsigned int idA = cached.collisionA.thisEntityID;
            unsigned int idB = cached.collisionB.thisEntityID;

            if (CollisionStayList.contains(idA)) { CollisionStayList[idA].Invoke(cached.collisionA); }
            if (CollisionStayList.contains(idB)) { CollisionStayList[idB].Invoke(cached.collisionB); }
        }
    }

    void PhysicsEventCallback::ProcessTriggerStay() {
        for (const auto& pair : m_activeTriggers) {
            unsigned int entityA = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.trigger->userData));
            unsigned int entityB = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.other->userData));

            Collision triggerA{}, triggerB{};
            triggerA.thisEntityID = entityA;
            triggerA.otherEntityID = entityB;
            triggerB.thisEntityID = entityB;
            triggerB.otherEntityID = entityA;

            if (TriggerStayList.contains(entityA)) { TriggerStayList[entityA].Invoke(triggerA); }
            if (TriggerStayList.contains(entityB)) { TriggerStayList[entityB].Invoke(triggerB); }
        }
    }

    void PhysicsEventCallback::DeregisterEntity(unsigned int id) {
        CollisionEnterList.erase(id);
        CollisionStayList.erase(id);
        CollisionExitList.erase(id);
        TriggerEnterList.erase(id);
        TriggerStayList.erase(id);
        TriggerExitList.erase(id);
        std::erase_if(m_activeCollisions, [&](const CollisionPair& pair) {
            unsigned int a = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.collision->userData));
            unsigned int b = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.other->userData));
            return a == id || b == id;
        });
        std::erase_if(m_cachedCollisionData, [&](const auto& kv) {
            unsigned int a = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(kv.first.collision->userData));
            unsigned int b = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(kv.first.other->userData));
            return a == id || b == id;
        });
        std::erase_if(m_activeTriggers, [&](const TriggerPair& pair) {
            unsigned int a = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.trigger->userData));
            unsigned int b = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pair.other->userData));
            return a == id || b == id;
        });
        std::erase_if(m_pendingCollisionEvents, [&](const PendingCollisionEvent& e) {
            return e.collisionA.thisEntityID == id || e.collisionB.thisEntityID == id;
        });
        std::erase_if(m_pendingTriggerEvents, [&](const PendingTriggerEvent& e) {
            return e.entityA == id || e.entityB == id;
        });
    }
}