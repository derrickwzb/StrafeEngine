

#include "strafepch.h"

#include "Application.h"

#include "Core/Logger.h"
#include "Core/Core.h"
#include "Entity/EntityManager.h"
#include "Graphics/Window/Window.h"
#include "Graphics/GLFWContext.h"
#include "Graphics/OpenGLContext.h"
#include "Event/WindowEvents.h"
#include "Event/KeyEvents.h"
#include "Event/MouseEvent.h"
#include "glad/glad.h"
#include "Input/InputHandler.h"
#include "Graphics/Renderer/RenderGraph.h"
#include "Graphics/Transform/Transform.h"
#include "Layer/LayerStack.h"
#include "Graphics/Transform/TransformLayer.h"
#include "Resource/ResourceManager.h"
#include "File/FileManager.h"

#include "Strafe/Graphics/Renderer/RenderData.h"
#include "Strafe/Core/Threading/GenericThread.h"
#include "Strafe/Core/Threading/Runnable.h"
#include "Strafe/Core/TaskGraph/TaskGraphInterface.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformMisc.h"

strafe::Transform* t1, *t2, *t3, *t4, *t5;

//static GraphEventArray event;

namespace strafe
{
	void Application::Init(const ApplicationConfig& config)
	{

		/*TaskGraphInterface::Startup(WindowsPlatformMisc::NumberOfWorkerThreadsToSpawn());
		TaskGraphInterface::Get().AttachToThread(NamedThreadsEnum::GameThread);*/
		//UNREFERENCED_PARAMETER(config);
		//Logger::Init();
		//RD_CORE_INFO("spdlog initialized for use.");

		//GLFWContext::Init();

		//m_PrimaryWindow = std::make_shared<Window>();
		//m_PrimaryWindow->Init();
		////bind the application callback to the window
		//m_PrimaryWindow->SetEventCallback(RD_BIND_EVENT_FN(Application::OnEvent));
		////setup opengl context
		//m_Context = std::make_shared<OpenGLContext>();
		//m_Context->Init(m_PrimaryWindow);
		////setup input handler
		//m_InputHandler = std::make_shared<InputHandler>();
		//m_InputHandler->Init();
		////create the entity manager
		//m_EntityManager = std::make_shared<EntityManager>();
		////create the resource manager
		//m_ResourceManager = std::make_shared<ResourceManager>();
		//m_ResourceManager->Init(m_PrimaryWindow);
		////create the render graph
		//m_RenderGraph = std::make_shared<RenderGraph>();
		//m_RenderGraph->Init(m_PrimaryWindow, m_ResourceManager, m_EntityManager);
		////create the file manager
		//m_FileManager = std::make_shared<FileManager>();
		//m_FileManager->Init();
		//RD_CORE_INFO("{}", m_FileManager->GetRoot().string());
		//Guid guid = GuidGenerator::GenerateGuid();
		//m_FileManager->QueueRequest(FileIORequest{ guid, "vertexshader.vtx", [](Guid id, const uint8_t* data, uint32_t size)
		//{
		//	//remember to null terminate the string since it is loaded in binary
		//	RD_CORE_TRACE("size:{} -> {}", size, std::string(reinterpret_cast<const char*>(data), size));
		//}});

		////layers stuff
		//m_LayerStack = std::make_shared<LayerStack>();
		////adding the transform layer
		//m_TransformLayer = std::make_shared<TransformLayer>(m_EntityManager);
		//m_LayerStack->PushLayer(m_TransformLayer);

		////init all layers
		//m_LayerStack->Init();

		////test the transform layer
		//auto e1 = m_EntityManager->CreateEntity();
		//auto e2 = m_EntityManager->CreateEntity();
		//auto e3 = m_EntityManager->CreateEntity();
		//auto e4 = m_EntityManager->CreateEntity();
		//auto e5 = m_EntityManager->CreateEntity();
		//t1 = m_EntityManager->AddComponent<Transform>(e1);
		//t1->m_LocalPosition = { 0.3f, 0.f, 0.f };
		//m_TransformLayer->SetEntityAsRoot(m_EntityManager->GetGuid(e1));
		//t2 = m_EntityManager->AddComponent<Transform>(e2);
		//t2->m_LocalPosition = { 0.3f, 0.f, 0.f };
		//t2->m_Parent = m_EntityManager->GetGuid(e1);
		//t1->m_Child = m_EntityManager->GetGuid(e2);
		//t3 = m_EntityManager->AddComponent<Transform>(e3);
		//t3->m_LocalPosition = { 0.3f, 0.f, 0.f };
		//t3->m_Parent = m_EntityManager->GetGuid(e2);
		//t2->m_Child = m_EntityManager->GetGuid(e3);
		//t4 = m_EntityManager->AddComponent<Transform>(e4);
		//t4->m_LocalPosition = { 0.f, 0.3f, 0.f };
		//t4->m_Parent = m_EntityManager->GetGuid(e1);
		//t2->m_Sibling = m_EntityManager->GetGuid(e4);
		//t5 = m_EntityManager->AddComponent<Transform>(e5);
		//t5->m_LocalPosition = { 0.f, 0.3f, 0.f };
		//t5->m_Parent = m_EntityManager->GetGuid(e4);
		//t4->m_Child = m_EntityManager->GetGuid(e5);

		////add all of this into the render graph data
		//std::vector<RenderData> renderData;
		//renderData.push_back({.m_DrawMesh{ m_EntityManager->GetGuid(e1), 0 }});
		//renderData.push_back({.m_DrawMesh{ m_EntityManager->GetGuid(e2), 0 }});
		//renderData.push_back({.m_DrawMesh{ m_EntityManager->GetGuid(e3), 0 }});
		//renderData.push_back({.m_DrawMesh{ m_EntityManager->GetGuid(e4), 0 }});
		//renderData.push_back({.m_DrawMesh{ m_EntityManager->GetGuid(e5), 0 }});
		//m_ResourceManager->AddRenderData("test", std::move(renderData));
		TaskGraphInterface::Startup(WindowsPlatformMisc::NumberOfWorkerThreadsToSpawn());
		TaskGraphInterface::Get().AttachToThread(NamedThreadsEnum::GameThread);

		std::cout << "Starting Performance Test" << std::endl;




		

		
		

		
	}

	static const signed int WORK_LOAD = 100000000;
	double PerformWork(signed int WorkLoad)
	{
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << "singlethreaded for loop" << std::endl;
		double Result = 0.0;
		for (signed int i = 0; i < WorkLoad; ++i)
		{
			//std::cout << "Singlethreaede id :" << std::this_thread::get_id() << std::endl;
			Result += std::sqrt(i * 1.0);
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> single_threaded_time = end - start;

		std::cout << "Single-threaded time: " << single_threaded_time.count() << " seconds" << std::endl;
		std::cout << "Single-threaded result: " << Result << std::endl;
		return Result;

	}


	double PerformWorkmt(signed int WorkLoad)
	{
		std::cout << "multithreaded for loop" << std::endl;

		auto start = std::chrono::high_resolution_clock::now();
		double Result = 0.0;
		for (signed int i = 0; i < WorkLoad; ++i)
		{
			Result += std::sqrt(i * 1.0);
		}
		
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> multi_threaded_time = end - start;

		std::cout << "Multi-threaded time: " << multi_threaded_time.count() << " seconds" << std::endl;
		return Result;
	}
	unsigned int ThreadWork()
	{
		double Result = PerformWorkmt(WORK_LOAD);
		std::cout << "multithreaded Thread result: " << Result << std::endl;
		return 0;
	}

	class FMyWorker : public Runnable
	{
	public:

		// The boolean that acts as the main switch
	// When this is false, inputs and outputs can be safely read from game thread
		bool bInputReady = false;


		// Declare the variables that are the inputs and outputs here.
		// You can have as many as you want. Remember not to use pointers, this must be
		// plain old data
		// For example:
		int ExampleIntInput = 0;
		float ExampleFloatOutput = 0.0f;
		// Constructor, create the thread by calling this
		FMyWorker()
		{
			// Constructs the actual thread object. It will begin execution immediately
			// If you've passed in any inputs, set them up before calling this.
			Thread = GenericThread::Create(this, TEXT("Give your thread a good name"), 0U, ThreadPri_Highest);
		}

		// Destructor
		virtual ~FMyWorker() override
		{
			if (Thread)
			{
				// Kill() is a blocking call, it waits for the thread to finish.
				// Hopefully that doesn't take too long
				Thread->Kill();
				delete Thread;
			}
		}


		// Overriden from FRunnable
		// Do not call these functions youself, that will happen automatically
		bool Init() override
		{
			std::cout << "My custom thread has been initialized" << std::endl;

				// Return false if you want to abort the thread
				return true;
		}// Do your setup here, allocate memory, ect.
		uint32 Run() override
		{
			while (1)
			{
				Sleep(5000);
			}
			// Peform your processor intensive task here. In this example, a neverending
			// task is created, which will only end when Stop is called.
				
				ThreadWork();
			

			return 0;
		}// Main data processing happens here
		void Stop() override
		{
			// Clean up memory usage here, and make sure the Run() function stops soon
			// The main thread will be stopped until this finishes!

			// For this example, we just need to terminate the while loop
			// It will finish in <= 1 sec, due to the Sleep()
			bRunThread = false;
		}// Clean up any memory you allocated here


	private:

		// Thread handle. Control the thread using this, with operators like Kill and Suspend
		GenericThread* Thread;

		// Used to know when the thread should exit, changed in Stop(), read in Run()
		bool bRunThread;
	};

	
	void TestFunction()
	{
		auto start = std::chrono::high_resolution_clock::now();
		FMyWorker* Worker1 = new FMyWorker();
		FMyWorker* Worker2 = new FMyWorker();
		std::cout << "Starting Performance Test" << std::endl;

		// Single-threaded test
		
		double single_result = PerformWork(WORK_LOAD);
		
		

		// Multi-threaded test
		//start = std::chrono::high_resolution_clock::now();

		//if (Worker1)
		//{

		//	
		//	Worker1->Run();
		//	//PerformWork(WORK_LOAD);
		//	//Worker2->Run();
		//	
		//	//std::cout << "this might run first" << std::endl;
		//}

		

		delete Worker1;
		delete Worker2;

		/*double speedup = single_threaded_time.count() / multi_threaded_time.count();
		std::cout << "Speedup: " << speedup << std::endl;*/
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Total time: " << std::chrono::duration<double>(end - start).count() << " seconds :: FUNCTION ENDS HERE" << std::endl;
	}


	class GenericTask
	{
		int	SomeArgument;
	public:
		GenericTask(int InSomeArgument) // CAUTION!: Must not use references in the constructor args; use pointers instead if you need by reference
			: SomeArgument(InSomeArgument)
		{
			// Usually the constructor doesn't do anything except save the arguments for use in DoWork or GetDesiredThread.
		}
		~GenericTask()
		{
			// you will be destroyed immediately after you execute. Might as well do cleanup in DoWork, but you could also use a destructor.
		}

		static NamedThreadsEnum::Type GetDesiredThread()
		{
			return NamedThreadsEnum::GameThread;
		}
		static SubsequentsModeEnum::Type GetSubsequentsMode()
		{
			return SubsequentsModeEnum::TrackSubsequents;
		}
		void DoTask(NamedThreadsEnum::Type CurrentThread, const GraphEventRef& MyCompletionGraphEvent)
		{
			// The arguments are useful for setting up other tasks.
			// Do work here, probably using SomeArgument.
			// 
			//MyCompletionGraphEvent->DontCompleteUntil(TGraphTask<SomeChildTask>::CreateTask(NULL, CurrentThread).ConstructAndDispatchWhenReady());
			std::cout << "test"<<SomeArgument<<std::endl;
		}
	};
	bool once = false;
	void Application::Run()
	{
		

		while(m_Running)
		{
			//
			////input update must be called before window polls for inputs
			//m_InputHandler->Update(m_PrimaryWindow->GetDeltaTime());
			//m_FileManager->Update();
			//m_PrimaryWindow->StartRender();

			//for(auto& layer : *m_LayerStack)
			//{
			//	layer->Update(static_cast<float>(m_PrimaryWindow->GetDeltaTime()));
			//}

			//m_RenderGraph->Execute();

			//m_PrimaryWindow->EndRender();
			/*TaskGraphInterface::Startup(WindowsPlatformMisc::NumberOfWorkerThreadsToSpawn());
			TaskGraphInterface::Get().AttachToThread(NamedThreadsEnum::GameThread);*/
			if (!once)
			{
				

				
				////FMyWorker* Worker2 = new FMyWorker();
				//once = true;
			

				GraphEventArray event;
				//event.push_back( FunctionGraphTask::CreateAndDispatchWhenReady([]()
				//	{
				//		std::cout << "fromgraphtask2";

				//	}, NULL, NamedThreadsEnum::BackgroundThreadPriority));

				GraphEventRef task1 = TGraphTask<GenericTask>::CreateTask(NULL, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(123);
				event.push_back(task1);
				GraphEventRef task2 = TGraphTask<GenericTask>::CreateTask(NULL, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(123434);
				event.push_back(task2);
				
				//TaskGraphInterface::Get().WaitUntilTaskCompletes((TGraphTask<GenericTask>::CreateTask(&event, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(77788)));

				GraphEventRef final = TGraphTask<GenericTask>::CreateTask(&event, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(77788);
				GraphEventArray event2;
				event2.push_back(final);

				TaskGraphInterface::Get().WaitUntilTaskCompletes(TGraphTask<GenericTask>::CreateTask(&event2, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(12345678));
				
				//event.push_back(final);
				//TGraphTask<GenericTask>::CreateTask(&event, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(77788);
				//TGraphTask<NullGraphTask>::CreateTask(&event, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(NamedThreadsEnum::GameThread);
				//event.push_back(task2);
				//TGraphTask<NullGraphTask>::CreateTask(&event, NamedThreadsEnum::GameThread).ConstructAndDispatchWhenReady(NamedThreadsEnum::GameThread);
				//TaskGraphInterface::Get().WaitUntilTaskCompletes(FunctionGraphTask::CreateAndDispatchWhenReady([]()
				//	{
				//		std::cout << "fromgraphtask3";
				//		//TaskGraphInterface::Get().requestquit(NamedThreadsEnum::GameThread);
				//		/*FunctionGraphTask::CreateAndDispatchWhenReady([]()
				//			{
				//				std::cout << "fromgraphtask2";

				//			}, NULL);*/

				//	}, NULL));
				//final->Wait();
				TaskGraphInterface::Get().ProcessThreadUntilIdle(NamedThreadsEnum::GameThread);
				once = true;
				
				
			}

			std::cout << "here";
			
				//TaskGraphInterface::Get().WaitUntilTaskCompletes(FunctionGraphTask::CreateAndDispatchWhenReady([]()
				//	{
				//		std::cout << "fromgraphtask3";
				//		//TaskGraphInterface::Get().requestquit(NamedThreadsEnum::GameThread);
				//		/*FunctionGraphTask::CreateAndDispatchWhenReady([]()
				//			{
				//				std::cout << "fromgraphtask2";
				//				
				//			}, NULL);*/

				//	}, NULL));

				


				//std::cout << "here";

		}
	}

	void Application::Shutdown()
	{
		
		m_PrimaryWindow->Shutdown();
		GLFWContext::Shutdown();
		RD_CORE_INFO("ragdoll Engine application shut down successfull");
		TaskGraphInterface::Get().Shutdown();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		RD_DISPATCH_EVENT(dispatcher, WindowCloseEvent, event, Application::OnWindowClose);
		RD_DISPATCH_EVENT(dispatcher, WindowResizeEvent, event, Application::OnWindowResize);
		RD_DISPATCH_EVENT(dispatcher, WindowMoveEvent, event, Application::OnWindowMove);
		RD_DISPATCH_EVENT(dispatcher, KeyPressedEvent, event, Application::OnKeyPressed);
		RD_DISPATCH_EVENT(dispatcher, KeyReleasedEvent, event, Application::OnKeyReleased);
		RD_DISPATCH_EVENT(dispatcher, KeyTypedEvent, event, Application::OnKeyTyped);
		RD_DISPATCH_EVENT(dispatcher, MouseMovedEvent, event, Application::OnMouseMove);
		RD_DISPATCH_EVENT(dispatcher, MouseButtonPressedEvent, event, Application::OnMouseButtonPressed);
		RD_DISPATCH_EVENT(dispatcher, MouseButtonReleasedEvent, event, Application::OnMouseButtonReleased);
		RD_DISPATCH_EVENT(dispatcher, MouseScrolledEvent, event, Application::OnMouseScrolled);

		//run the event on all layers
		for(auto& layer : *m_LayerStack)
		{
			layer->OnEvent(event);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		UNREFERENCED_PARAMETER(event);
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		UNREFERENCED_PARAMETER(event);
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		return false;
	}

	bool Application::OnWindowMove(WindowMoveEvent& event)
	{
		UNREFERENCED_PARAMETER(event);
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		return false;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnKeyPressed(event);
		return false;
	}

	bool Application::OnKeyReleased(KeyReleasedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnKeyReleased(event);
		return false;
	}

	bool Application::OnKeyTyped(KeyTypedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnKeyTyped(event);
		return false;
	}

	bool Application::OnMouseMove(MouseMovedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnMouseMove(event);
		return false;
	}

	bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnMouseButtonPressed(event);
		return false;
	}

	bool Application::OnMouseButtonReleased(MouseButtonReleasedEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnMouseButtonReleased(event);
		return false;
	}

	bool Application::OnMouseScrolled(MouseScrolledEvent& event)
	{
#if RD_LOG_EVENT
		RD_CORE_TRACE("Event: {}", event.ToString());
#endif
		m_InputHandler->OnMouseScrolled(event);
		return false;
	}
}
