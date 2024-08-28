
#pragma once

namespace strafe
{
	class Event;
	class EntityManager;

	class Layer
	{
	public:
		Layer(const char* name) : m_DebugName{ name } {};
		virtual ~Layer() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void PreUpdate(float _dt);
		virtual void Update(float _dt) = 0;
		virtual void PostUpdate(float _dt);
		virtual void OnEvent(Event& e);

		bool IsEnabled() const { return m_Enabled; }
		void SetEnabled(bool enabled) { m_Enabled = enabled; }
		const char* GetDebugName() const { return m_DebugName; }

	protected:
		bool m_Enabled = true;

		const char* m_DebugName;
	};
}
