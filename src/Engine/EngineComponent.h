//
// Created by Simon Cros on 1/27/25.
//

#ifndef ENGINECOMPONENT_H
#define ENGINECOMPONENT_H

class Engine;
class Object;

class EngineComponent
{
protected:
    Object& m_object;

public:
    explicit EngineComponent(Object& object) : m_object(object)
    {
    }

    virtual ~EngineComponent() = default;

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

#endif //ENGINECOMPONENT_H
