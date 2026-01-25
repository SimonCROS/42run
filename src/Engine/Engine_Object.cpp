//
// Created by Simon Cros on 1/29/25.
//

module Engine;
import std;
import glm;

auto Object::willUpdate(Engine& engine) -> void
{
    for (auto& component : m_components)
        component->onWillUpdate(engine);
}

auto Object::update(Engine& engine) -> void
{
    for (auto& component : m_components)
        component->onUpdate(engine);
}

auto Object::render(Engine& engine) -> void
{
    for (auto& component : m_components)
        component->onRender(engine);
}

auto Object::postRender(Engine& engine) -> void
{
    for (auto& component : m_components)
        component->onPostRender(engine);
}

auto Object::markDirty() -> void
{
    if (!m_dirty)
    {
        m_dirty = true;

        auto child = m_firstChildIndex;
        while (child != SlotSetIndex::invalid())
        {
            m_engine.get().objects()[child].markDirty();
            child = m_engine.get().objects()[child].m_nextSiblingIndex;
        }
    }
}

auto Object::updateWorldTransformIfDirty() -> void
{
    if (m_dirty)
    {
        if (m_parentIndex != SlotSetIndex::invalid())
        {
            auto& parent = m_engine.get().objects()[m_parentIndex];
            parent.updateWorldTransformIfDirty();
            m_worldTransform = parent.m_worldTransform * m_transform.trs();
        }
        else
        {
            m_worldTransform = m_transform.trs();
        }

        m_dirty = false;
    }
}

auto Object::setActiveFromParent(const bool active) -> void
{
    m_isParentActive = active;

    if (m_isActive) // stop propagation if already disabled
    {
        auto child = m_firstChildIndex;
        while (child != SlotSetIndex::invalid())
        {
            m_engine.get().objects()[child].setActiveFromParent(active);
            child = m_engine.get().objects()[child].m_nextSiblingIndex;
        }
    }
}

auto Object::setActive(const bool active) -> void
{
    if (m_isActive != active)
    {
        m_isActive = active;

        auto child = m_firstChildIndex;
        while (child != SlotSetIndex::invalid())
        {
            m_engine.get().objects()[child].setActiveFromParent(active);
            child = m_engine.get().objects()[child].m_nextSiblingIndex;
        }
    }
}

auto Object::unsetParentInternal(const bool recursiveUpdate) -> void
{
    if (m_parentIndex == SlotSetIndex::invalid())
        return;

    auto* prevNextPtr = &m_engine.get().objects()[m_parentIndex].m_firstChildIndex;
    auto current = *prevNextPtr;

    while (current != SlotSetIndex::invalid())
    {
        if (current == index)
        {
            *prevNextPtr = m_nextSiblingIndex;
            break;
        }
        prevNextPtr = &m_engine.get().objects()[current].m_nextSiblingIndex;
        current = m_engine.get().objects()[current].m_nextSiblingIndex;
    }

    m_parentIndex = SlotSetIndex::invalid();
    m_nextSiblingIndex = SlotSetIndex::invalid();

    if (recursiveUpdate)
    {
        setActiveFromParent(isActiveSelf());
        markDirty();
    }
}

auto Object::setParent(Object& object) -> void
{
    assert(index != object.index);
    if (m_parentIndex == object.index)
        return;

    unsetParentInternal(false);

    m_parentIndex = object.index;
    m_nextSiblingIndex = object.m_firstChildIndex;
    object.m_firstChildIndex = index;

    setActiveFromParent(object.isActive());
    markDirty();
}

auto Object::unsetParent() -> void
{
    unsetParentInternal(true);
}
