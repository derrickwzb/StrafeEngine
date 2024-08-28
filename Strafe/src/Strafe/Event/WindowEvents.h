
#pragma once

#include "Event.h"

namespace strafe
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int32_t _width, int32_t _height)
			: m_Width(_width), m_Height(_height) {}

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		// Sets the types and category for the event
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int32_t m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowMoveEvent : public Event
	{
	public:
		WindowMoveEvent(int32_t _x, int32_t _y)
			: m_PosX(_x), m_PosY(_y) {}

		inline int32_t GetX() const { return m_PosX; }
		inline int32_t GetY() const { return m_PosY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMoveEvent: " << m_PosX << ", " << m_PosY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		int32_t m_PosX, m_PosY;
	};
}
