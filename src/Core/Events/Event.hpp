#pragma once

#include <string>
#include <functional>
#include <ostream>
#include <concepts>

#include "Types.hpp"

#define BIT(x) (1 << x)

namespace Graphics {

    enum class EventType
    {
        None = 0,
		WindowClose, WindowMinimize, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseButtonDown, MouseMoved, MouseScrolled,
    };

    enum EventCategory
    {
        None = 0,
		EventCategoryApplication    = BIT(0),
		EventCategoryInput          = BIT(1),
		EventCategoryKeyboard       = BIT(2),
		EventCategoryMouse          = BIT(3),
		EventCategoryMouseButton    = BIT(4),
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual i32 GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		bool Handled = false;

		virtual ~Event() = default;
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual i32 GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) const
		{
			return GetCategoryFlags() & category;
		}
	};

	template<typename T>
	concept IsEvent = requires(T e)
	{
		{ T::GetStaticType() } -> std::same_as<EventType>;
		{ e.GetEventType() } -> std::same_as<EventType>;
		{ e.GetName() } -> std::convertible_to<const char*>;
		{ e.GetCategoryFlags() } -> std::convertible_to<i32>;
		{ e.ToString() } -> std::convertible_to<std::string>;
		requires std::derived_from<T, Event>;
	};

	class EventDispatcher
	{
		template<IsEvent T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		template<IsEvent T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled)
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
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
