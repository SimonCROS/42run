//
// Created by Simon Cros on 1/29/25.
//

export module Engine:Object;
import :Transform;
import :Component;
import std;
import glm;
import Utility;

export class Engine;

export class Object
{
    friend class Engine;
    friend class Transform;

public:
    SlotSetIndex index;

private:
    std::reference_wrapper<Engine> m_engine;
    Transform m_transform;

    bool m_dirty{true};

    bool m_isActive{true};
    bool m_isParentActive{true};

    SlotSetIndex m_parentIndex;
    SlotSetIndex m_firstChildIndex;
    SlotSetIndex m_nextSiblingIndex;

    glm::mat4 m_worldTransform{};

    std::unordered_set<std::unique_ptr<Component>> m_components;

    auto willUpdate(Engine& engine) -> void;
    auto update(Engine& engine) -> void;
    auto render(Engine& engine) -> void;
    auto postRender(Engine& engine) -> void;

    auto markDirty() -> void;
    auto updateWorldTransformIfDirty() -> void;

    auto setActiveFromParent(bool active) -> void;

public:
    explicit Object(Engine& engine)
        : m_engine(engine), m_transform(*this)
    {
    }

    [[nodiscard]] auto transform() -> Transform& { return m_transform; }
    [[nodiscard]] auto transform() const -> const Transform& { return m_transform; }

    [[nodiscard]] auto isActiveSelf() const -> bool { return m_isActive; }
    [[nodiscard]] auto isActive() const -> bool { return isActiveSelf() && m_isParentActive; }

    auto setActive(bool active) -> void;

    template <class T, class... Args>
        requires std::derived_from<T, Component> && std::constructible_from<T, Object&, Args...>
    auto addComponent(Args&&... args) -> T&
    {
        return dynamic_cast<T&>(**m_components.emplace(std::make_unique<T>(
            *this, std::forward<Args>(args)...)).first);
    }

    template <class T>
        requires std::derived_from<T, Component>
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

    auto setParent(Object& object) -> void;
    auto unsetParent() -> void;

    auto worldTransform() -> glm::mat4
    {
        updateWorldTransformIfDirty();
        return m_worldTransform;
    }

private:
    auto unsetParentInternal(bool recursiveUpdate) -> void;

    friend auto swap(Object& a, Object& b) noexcept -> void
    {
        std::swap(a.index, b.index);
        std::swap(a.m_engine, b.m_engine);
        std::swap(a.m_transform, b.m_transform);
        std::swap(a.m_dirty, b.m_dirty);
        std::swap(a.m_isActive, b.m_isActive);
        std::swap(a.m_isParentActive, b.m_isParentActive);
        std::swap(a.m_parentIndex, b.m_parentIndex);
        std::swap(a.m_firstChildIndex, b.m_firstChildIndex);
        std::swap(a.m_nextSiblingIndex, b.m_nextSiblingIndex);
        std::swap(a.m_worldTransform, b.m_worldTransform);
        std::swap(a.m_components, b.m_components);
    }
};
