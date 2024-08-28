
#pragma once
#include "Strafe/Event/EventEnums.h"

namespace strafe
{
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
		friend class EventDispatcher;
	public:
		virtual ~Event() = default;

		bool m_Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int32_t GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }
		bool IsInCategory(EventCategory category) const
		{
			return GetCategoryFlags() & category;
		}
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		template<typename T>
		using EventFn = std::function<bool(T&)>;
		/**
		 * \brief This function will check if the templated type is the same as the event type
		 * If the event type is the same, the dispatcher will execute the callback function.
		 * \tparam T Type of event
		 * \param func Event callback function
		 * \return True if callback was executed, false if it was not.
		 */
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				if (m_Event.m_Handled)
					return false;
				m_Event.m_Handled = func(*static_cast<T*>(&m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}

#define RD_CONCAT(a, b) RD_CONCAT_INNER(a, b)
#define RD_CONCAT_INNER(a, b) a ## b

#define RD_DISPATCH_EVENT(dispatcher, eventType, event, callback)\
static auto RD_CONCAT(var, __LINE__) = RD_BIND_EVENT_FN(callback); dispatcher.Dispatch<eventType>(RD_CONCAT(var, __LINE__))