//
// Created by Simon Cros on 1/27/25.
//

export module Engine:Component;

class Engine;
class Object;

export class Component
{
protected:
    Object& m_object;

public:
    explicit Component(Object& object) : m_object(object)
    {
    }

    virtual ~Component() = default;

    [[nodiscard]] auto object() -> Object& { return m_object; }
    [[nodiscard]] auto object() const -> const Object& { return m_object; }

    virtual auto onWillUpdate(Engine& engine) -> void
    {
    }

    virtual auto onUpdate(Engine& engine) -> void
    {
    }

    virtual auto onRender(Engine& engine) -> void
    {
    }

    virtual auto onPostRender(Engine& engine) -> void
    {
    }
};
