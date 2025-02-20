//
// Created by Simon Cros on 1/29/25.
//

#include "Object.h"
#include "EngineComponent.h"

auto Object::willUpdate(Engine& engine) const -> void
{
    for (auto& component : m_components)
        component->onWillUpdate(engine);
}

auto Object::update(Engine& engine) const -> void
{
    for (auto& component : m_components)
        component->onUpdate(engine);
}

auto Object::render(Engine& engine) const -> void
{
    for (auto& component : m_components)
        component->onRender(engine);
}

auto Object::postRender(Engine& engine) const -> void
{
    for (auto& component : m_components)
        component->onPostRender(engine);
}
