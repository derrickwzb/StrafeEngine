
#pragma once

namespace strafe
{
	struct IResource	//interface to share common functionality between different resource types
	{
		Guid m_Guid;
		//the dependencies it has
		std::vector<Guid> m_Dependencies;
	};

	template<typename T>
	struct Resource : IResource
	{
		inline const static std::shared_ptr<T> null{ nullptr };
		//pointer to the actual resource type
		std::shared_ptr<T> m_Data;

		//this will only take bytes to load into the resource
		bool Load(const char* data, uint32_t size)
		{
			//behavior on what to do will be up to constructors
			m_Data = std::make_shared<T>(data, size);
		}
		//this will free the resource`
		bool Unload()
		{
			//behavior on what to do will be up to destructors
			m_Data.reset();
		}
	};
}
