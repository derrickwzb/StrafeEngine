
#pragma once
#include "Core/Guid.h"
#include <iostream>

namespace strafe
{
	class EntityManager;
	class TransformLayer;
	class RenderGraph;
	class OpenGLContext;
	class ResourceManager;
	class WindowMoveEvent;
	class WindowResizeEvent;
	class WindowCloseEvent;
	class KeyPressedEvent;
	class KeyReleasedEvent;
	class KeyTypedEvent;
	class MouseMovedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
	class MouseScrolledEvent;
	class Event;
	class Window;
	class LayerStack;
	class FileManager;
	class InputHandler;

	class Application
	{
	public:
		struct ApplicationConfig
		{
			
		};

		Application() = default;
		virtual ~Application() = default;

		virtual void Init(const ApplicationConfig& config);
		void Run();
		virtual void Shutdown();

		void OnEvent(Event& event);

		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnWindowMove(WindowMoveEvent& event);

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnKeyReleased(KeyReleasedEvent& event);
		bool OnKeyTyped(KeyTypedEvent& event);

		bool OnMouseMove(MouseMovedEvent& event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
		bool OnMouseScrolled(MouseScrolledEvent& event);

	protected:
		bool m_Running{ true };

		GuidGenerator m_GuidGenerator{};
		std::shared_ptr<Window> m_PrimaryWindow;
		std::shared_ptr<OpenGLContext> m_Context;
		std::shared_ptr<InputHandler> m_InputHandler;
		std::shared_ptr<EntityManager> m_EntityManager;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<RenderGraph> m_RenderGraph;
		std::shared_ptr<FileManager> m_FileManager;

		std::shared_ptr<LayerStack> m_LayerStack;
		std::shared_ptr<TransformLayer> m_TransformLayer;
	};

	/**
	 * \brief Function to be defined by the client in the editor or launcher project. This way the engine will run a modified version of the application with the modifications needed by the client.
	 * \return The application created
	 */
	Application* CreateApplication();
}
