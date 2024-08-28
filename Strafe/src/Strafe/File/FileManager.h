﻿
#pragma once

#include "Strafe/Memory/StrafeAllocator.h"

namespace strafe
{
	struct FileIORequest
	{
		enum class Type
		{
			Read,
			Write
		};
		enum class Priority
		{
			High = 0,
			Normal,
			Low
		};

		FileIORequest()
		{
			if(m_Guid == Guid::null)
				m_Guid = GuidGenerator::GenerateGuid();
		}
		FileIORequest(Guid guid, std::filesystem::path path, std::function<void(Guid, const uint8_t*, uint32_t)> callback, uint64_t offset = 0, uint64_t size = 0, Type type = Type::Read, Priority priority = Priority::Normal)
			: m_Guid{ guid }, m_Path(path), m_ReadCallback(callback), m_Offset(offset), m_Size(size), m_Type(type), m_Priority(priority)
		{
			if (m_Guid == Guid::null)
				RD_ASSERT(true, "Please generate a guid for your request and keep track of it");
		}
		FileIORequest(Guid guid, std::filesystem::path path, uint8_t* data, uint32_t size, Type type = Type::Write, Priority priority = Priority::Normal)
			: m_Guid{ guid }, m_Path(path), m_WriteData(data), m_WriteSize(size), m_Type(type), m_Priority(priority)
		{
			if (m_Guid == Guid::null)
				RD_ASSERT(true, "Please generate a guid for your request and keep track of it");
		}
		FileIORequest(FileIORequest&&) = default;
		FileIORequest(const FileIORequest&) = default;
		~FileIORequest() = default;

		Guid m_Guid;
		Type m_Type;
		Priority m_Priority{ 0 };
		std::filesystem::path m_Path;
		uint64_t m_Offset{ 0 };
		uint64_t m_Size{ 0 };
		//id of the request, data ptr, and size of data for read
		std::function<void(Guid, const uint8_t*, uint32_t)> m_ReadCallback;
		//data ptr and size for write
		uint8_t* m_WriteData{ nullptr };
		uint32_t m_WriteSize{ 0 };

		FileIORequest& operator=(FileIORequest&& other) noexcept;
		FileIORequest& operator=(const FileIORequest& other);
	};

	class FileManager
	{
		struct Buffer
		{
			enum class Status
			{
				Idle,
				Executing,
				Callback,
			} m_Status{ Status::Idle };
			FileIORequest m_Request;
			std::vector<uint8_t, StrafeAllocator<uint8_t>> m_Data;

			void Load(std::filesystem::path root);
		};
	public:
		void Init();
		//checks when the file manager is done loading then can call the callbacks
		void Update();
		//check queue status and will load async, callback will be called when done in main thread
		void ThreadUpdate();
		void QueueRequest(FileIORequest request);

		void Shutdown();

		std::filesystem::path GetRoot() const { return m_Root; }

	private:
		//root directory
		std::filesystem::path m_Root = std::filesystem::current_path();
		//stop thread bool
		bool m_Running{ true };
		//queue of files to load
		std::priority_queue<FileIORequest, std::vector<FileIORequest>,
			std::function<bool(const FileIORequest&, const FileIORequest&)>> m_RequestQueue{
				[](const FileIORequest& lhs, const FileIORequest& rhs) {
					return lhs.m_Priority < rhs.m_Priority;
				}
		};
		//mutex for the queue
		std::mutex m_QueueMutex;
		//double buffering loading system with a loader thread
		Buffer m_Buffer[2]{};
		//thread to do IO
		std::thread m_IOThread;
	};
}
