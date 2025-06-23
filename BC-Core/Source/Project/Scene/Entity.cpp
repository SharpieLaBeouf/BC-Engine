#include "BC_PCH.h"
#include "Entity.h"

namespace BC
{
    Entity::Entity() :
        m_EntityHandle(entt::null),
        m_Scene(nullptr)
    {

    }

    Entity::Entity(entt::entity handle, Scene* scene_ptr) :
        m_EntityHandle(handle),
        m_Scene(scene_ptr)
    {

    }
    
    void Entity::Destroy()
    {
        
    }

}