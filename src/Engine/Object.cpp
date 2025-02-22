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

auto Object::markDirty(SlotSet<Object>& objects) -> void
{
    if (!m_transform.m_dirty)
    {
        m_transform.m_dirty = true;

        auto child = m_firstChildIndex;
        while (child != ObjectNoneIndex)
        {
            objects[child].markDirty(objects);
            child = objects[child].m_nextSiblingIndex;
        }
    }
}

auto Object::updateWorldTransformIfDirty(SlotSet<Object>& objects) -> void
{
    // if (m_transform.m_dirty) // TODO
    {
        if (m_parentIndex != ObjectNoneIndex)
        {
            auto& parent = objects[m_parentIndex];
            parent.updateWorldTransformIfDirty(objects);
            m_worldTransform = parent.m_worldTransform * m_transform.trs();
        }
        else
        {
            m_worldTransform = m_transform.trs();
        }

        m_transform.m_dirty = false;
    }
}

auto Object::setActiveFromParent(const bool active, SlotSet<Object>& objects) -> void
{
    m_isParentActive = active;

    if (m_isActive) // stop propagation if already disabled
    {
        auto child = m_firstChildIndex;
        while (child != ObjectNoneIndex)
        {
            objects[child].setActiveFromParent(active, objects);
            child = objects[child].m_nextSiblingIndex;
        }
    }
}

auto Object::setActive(const bool active, SlotSet<Object>& objects) -> void
{
    if (m_isActive != active)
    {
        m_isActive = active;

        auto child = m_firstChildIndex;
        while (child != ObjectNoneIndex)
        {
            objects[child].setActiveFromParent(active, objects);
            child = objects[child].m_nextSiblingIndex;
        }
    }
}

auto Object::unsetParentInternal(SlotSet<Object>& objects, const bool recursiveUpdate) -> void
{
    if (m_parentIndex == ObjectNoneIndex)
        return;

    auto* prevNextPtr = &objects[m_parentIndex].m_firstChildIndex;
    auto current = *prevNextPtr;

    while (current != ObjectNoneIndex)
    {
        if (current == index)
        {
            *prevNextPtr = m_nextSiblingIndex;
            break;
        }
        prevNextPtr = &objects[current].m_nextSiblingIndex;
        current = objects[current].m_nextSiblingIndex;
    }

    m_parentIndex = ObjectNoneIndex;
    m_nextSiblingIndex = ObjectNoneIndex;

    if (recursiveUpdate)
    {
        setActiveFromParent(isActiveSelf(), objects);
        markDirty(objects);
    }
}

auto Object::setParent(Object& object, SlotSet<Object>& objects) -> void
{
    assert(index != object.index);
    if (m_parentIndex == object.index)
        return;

    unsetParentInternal(objects, false);

    m_parentIndex = object.index;
    m_nextSiblingIndex = object.m_firstChildIndex;
    object.m_firstChildIndex = index;

    setActiveFromParent(object.isActive(), objects);
    markDirty(objects);
}

auto Object::unsetParent(SlotSet<Object>& objects) -> void
{
    unsetParentInternal(objects, true);
}
