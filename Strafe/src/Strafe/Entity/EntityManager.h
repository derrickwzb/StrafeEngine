
#pragma once

#include "entt/entt.hpp"

namespace strafe
{
	class EntityManager
	{
	public:
		entt::registry& GetRegistry() { return m_Registry; }

		entt::entity CreateEntity();
		entt::entity GetEntity(const Guid& guid);
		Guid GetGuid(const entt::entity& entity);

		template<typename T>
		T* GetComponent(const Guid& guid)
		{
			auto entity = GetEntity(guid);
			if(entity == entt::null)
				return nullptr;
			if(m_Registry.all_of<T>(entity))
				return &m_Registry.get<T>(entity);
			RD_CORE_ERROR("Entity does not have component {}", typeid(T).name());
			return nullptr;
		}

		template<typename T>
		T* GetComponent(const entt::entity& entity)
		{
			if(entity == entt::null)
			{
				RD_CORE_ERROR("Entity in T* GetComponent(const entt::entity& entity) is entt::null");
				return nullptr;
			}
			if(m_Registry.all_of<T>(entity))
				return &m_Registry.get<T>(entity);
			RD_CORE_ERROR("Entity does not have component {}", typeid(T).name());
			return nullptr;
		}

		template<typename T>
		T* AddComponent(const Guid& guid)
		{
			auto entity = GetEntity(guid);
			if(entity == entt::null)
				return nullptr;
			if (m_Registry.all_of<T>(entity))
			{
				RD_CORE_ERROR("Entity {} already has component {}, attempted to double add", guid, typeid(T).name());
				return &m_Registry.get<T>(entity);
			}
			return &m_Registry.emplace<T>(entity);

		}

		template<typename T>
		T* AddComponent(const entt::entity& entity)
		{
			if(entity == entt::null)
			{
				RD_CORE_ERROR("Entity in T& AddComponent(const entt::entity& entity) is entt::null");
				return nullptr;
			}
			if(m_Registry.all_of<T>(entity))
			{
				RD_CORE_ERROR("Entity already has component {}, attempted to double add", typeid(T).name());
				return &m_Registry.get<T>(entity);
			}
			return &m_Registry.emplace<T>(entity);
		}

	private:
		entt::registry m_Registry;

		std::unordered_map<uint64_t, entt::entity> m_GuidToEntity;
		std::unordered_map<entt::entity, uint64_t> m_EntityToGuid;
	};
}