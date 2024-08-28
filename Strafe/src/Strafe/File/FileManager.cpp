
#include "strafepch.h"

#include "FileManager.h"

namespace strafe
{
	FileIORequest& FileIORequest::operator=(FileIORequest&& other) noexcept
	{
		if(this != &other)
		{
			m_Path = std::move(other.m_Path);
			m_Size = other.m_Size;
			m_ReadCallback = std::move(other.m_ReadCallback);
			m_Guid = other.m_Guid;
			m_Type = other.m_Type;
		}
		return *this;
	}

	FileIORequest& FileIORequest::operator=(const FileIORequest& other)
	{
		if (this != &other)
		{
			m_Path = other.m_Path;
			m_Size = other.m_Size;
			m_ReadCallback = other.m_ReadCallback;
			m_Guid = other.m_Guid;
			m_Type = other.m_Type;
		}
		return *this;
	}

	void FileManager::Buffer::Load(std::filesystem::path root)
	{
		m_Status = Status::Executing;
		std::filesystem::path path = root / m_Request.m_Path;
		if(!std::filesystem::exists(path))
		{
			//RD_ASSERT(true, "File {} not found", path.string());
			m_Status = Status::Idle;
			return;
		}
		//only load byte data for the engine to use
		std::ifstream file(root / m_Request.m_Path, std::ios::binary);
		if (!file.is_open())
		{
			RD_ASSERT(true, "File {} unable to be opened", path.string());
			m_Status = Status::Idle;
			return;
		}
		//if size is 0, read the whole file
		std::streamsize size = m_Request.m_Size;
		if (m_Request.m_Size == 0)
		{
			file.seekg(0, std::ios::end);
			size = file.tellg();
			file.seekg(0, std::ios::beg);
		}
		m_Data.resize(size);
		file.read(reinterpret_cast<char*>(m_Data.data()), size);
		m_Status = Status::Callback;
		//once done get main thread to call the callback
	}

	void FileManager::Init()
	{
		//TODO: should be config next time
		m_Root = m_Root.parent_path() / "build";
		//define the path to the asset folder
		std::filesystem::path assetFolderPath = m_Root / "assets";
		//check if the asset folder exists
		if (!std::filesystem::exists(assetFolderPath))
		{
			//if not, create the folder
			try
			{
				std::filesystem::create_directory(assetFolderPath);
				RD_CORE_INFO("Asset folder created at: {}", assetFolderPath.string());;
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				RD_CORE_INFO("Error creating asset folder: {}", e.what());
				return; //handle error as appropriate
			}
		}

		//set the root to the asset folder
		m_Root = assetFolderPath;

		//start the io thread
		m_Running = true;
		m_IOThread = std::thread(&FileManager::ThreadUpdate, this);
		m_IOThread.detach();
	}

	void FileManager::Update()
	{
		for(auto& buffer : m_Buffer)
		{
			if (buffer.m_Status == Buffer::Status::Callback)
			{
				if(buffer.m_Request.m_ReadCallback)
					buffer.m_Request.m_ReadCallback(buffer.m_Request.m_Guid, buffer.m_Data.data(), buffer.m_Data.size());
				buffer.m_Status = Buffer::Status::Idle;
			}
		}
	}

	void FileManager::ThreadUpdate()
	{
		while(true)
		{
			//check if there are any requests
			if (m_RequestQueue.empty())
			{
				// Sleep for a bit
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			// Get the next request
			FileIORequest request = m_RequestQueue.top();
			if(request.m_Type == FileIORequest::Type::Read)
			{
				// Check for available buffers
				Buffer* freeBuffer = nullptr;
				if (m_Buffer[0].m_Status == Buffer::Status::Idle)
				{
					freeBuffer = &m_Buffer[0];
				}
				else if (m_Buffer[1].m_Status == Buffer::Status::Idle)
				{
					freeBuffer = &m_Buffer[1];
				}


				if(freeBuffer)
				{
					// Load the file
					freeBuffer->m_Request = request;
					freeBuffer->Load(m_Root);
					std::lock_guard<std::mutex> lock(m_QueueMutex);
					m_RequestQueue.pop();
				}
				else
				{
					// Sleep for a bit
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
			}
			else
			{
				//just write as per normal

				//no callbacks for writing because it should be fire and forget
			}
		}
	}

	void FileManager::QueueRequest(FileIORequest request)
	{
		std::lock_guard<std::mutex>lock(m_QueueMutex);
		m_RequestQueue.push(request);
	}

	void FileManager::Shutdown()
	{
		m_Running = false;
	}
}
