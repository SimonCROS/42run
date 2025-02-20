//
// Created by Simon Cros on 1/29/25.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <memory>
#include <unordered_set>

#include "Transform.h"
#include "Utility/SlotSet.h"

class Engine;
class EngineComponent;

class Object
{
    friend class Engine;

public:
    SlotSetIndex index;

private:
    Transform m_transform{};
    std::unordered_set<std::unique_ptr<EngineComponent>> m_components;

    auto willUpdate(Engine& engine) const -> void;
    auto update(Engine& engine) const -> void;
    auto render(Engine& engine) const -> void;
    auto postRender(Engine& engine) const -> void;

public:
    [[nodiscard]] auto transform() -> Transform& { return m_transform; }
    [[nodiscard]] auto transform() const -> const Transform& { return m_transform; }

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
};

#endif //OBJECT_H
