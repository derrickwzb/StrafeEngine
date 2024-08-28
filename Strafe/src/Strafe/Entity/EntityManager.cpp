

#include "strafepch.h"

#include "EntityManager.h"

#include "Strafe/Core/Logger.h"
#include "Strafe/Core/Guid.h"

namespace strafe
{
	entt::entity EntityManager::CreateEntity()
	{
		entt::entity entity = m_Registry.create();
		auto guid = GuidGenerator::GenerateGuid();
		m_GuidToEntity.insert({ guid, entity });
		m_EntityToGuid.insert({ entity, guid });
		return entity;
	}

	entt::entity EntityManager::GetEntity(const Guid& guid)
	{
		if(m_GuidToEntity.find(guid) != m_GuidToEntity.end())
			return m_GuidToEntity[guid];
		RD_CORE_WARN("Entity {} does not exist, returning entt::null");
		return entt::null;
	}

	Guid EntityManager::GetGuid(const entt::entity& entity)
	{
		if(m_EntityToGuid.find(entity) != m_EntityToGuid.end())
			return m_EntityToGuid[entity];
		RD_CORE_WARN("Entity does not exist, returning Guid::null");
		return 0;
	}
}
