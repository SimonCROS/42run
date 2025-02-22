//
// Created by Simon Cros on 1/29/25.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <memory>
#include <unordered_set>

#include "EngineComponent.h"
#include "Transform.h"
#include "Utility/SlotSet.h"

class Engine;
class EngineComponent;

constexpr SlotSetIndex ObjectNoneIndex = -1;

class Object
{
    friend class Engine;

public:
    SlotSetIndex index{ObjectNoneIndex};

private:
    Engine& m_engine;

    bool m_isActive{true};
    bool m_isParentActive{true};

    SlotSetIndex m_parentIndex{ObjectNoneIndex};
    SlotSetIndex m_firstChildIndex{ObjectNoneIndex};
    SlotSetIndex m_nextSiblingIndex{ObjectNoneIndex};

    Transform m_transform{};

    glm::mat4 m_worldTransform{};

    std::unordered_set<std::unique_ptr<EngineComponent>> m_components;

    auto willUpdate(Engine& engine) -> void;
    auto update(Engine& engine) -> void;
    auto render(Engine& engine) -> void;
    auto postRender(Engine& engine) -> void;

    auto markDirty() -> void;
    auto updateWorldTransformIfDirty() -> void;

    auto setActiveFromParent(bool active) -> void;

public:
    explicit Object(Engine& engine)
        : m_engine(engine)
    {
    }

    [[nodiscard]] auto transform() -> Transform& { return m_transform; }
    [[nodiscard]] auto transform() const -> const Transform& { return m_transform; }

    [[nodiscard]] auto isActiveSelf() const -> bool { return m_isActive; }
    [[nodiscard]] auto isActive() const -> bool { return isActiveSelf() && m_isParentActive; }

    auto setActive(bool active) -> void;

    template <class T, class... Args>
        requires std::derived_from<T, EngineComponent> && std::constructible_from<T, Object&, Args...>
    auto addComponent(Args&&... args) -> T&
    {
        return dynamic_cast<T&>(**m_components.emplace(std::make_unique<T>(
            *this, std::forward<Args>(args)...)).first);
    }

    template <class T>
        requires std::derived_from<T, EngineComponent>
    auto getComponent() -> std::optional<std::reference_wrapper<T>>
    {
        for (auto& component : m_components)
        {
            if (auto t = dynamic_cast<T*>(component.get()))
            {
                return *t;
            }
        }
        return std::nullopt;
    }

private:
    auto unsetParentInternal(bool recursiveUpdate) -> void;

public:
    auto setParent(Object& object) -> void;
    auto unsetParent() -> void;

    auto worldTransform() -> glm::mat4
    {
        updateWorldTransformIfDirty();
        return m_worldTransform;
    }
};

#endif //OBJECT_H
