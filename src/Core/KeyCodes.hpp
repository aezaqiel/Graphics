#pragma once

#include <ostream>

#include "Types.hpp"

namespace Graphics {

	typedef enum class KeyCode : u16
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	enum class KeyState
	{
		None = -1,
		Pressed,
		Held,
		Released
	};

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	typedef enum class MouseButton : u16
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;


	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<i32>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseButton button)
	{
		os << static_cast<i32>(button);
		return os;
	}

}

// From glfw3.h
#define KEY_SPACE           ::Graphics::Key::Space
#define KEY_APOSTROPHE      ::Graphics::Key::Apostrophe    /* ' */
#define KEY_COMMA           ::Graphics::Key::Comma         /* , */
#define KEY_MINUS           ::Graphics::Key::Minus         /* - */
#define KEY_PERIOD          ::Graphics::Key::Period        /* . */
#define KEY_SLASH           ::Graphics::Key::Slash         /* / */
#define KEY_0               ::Graphics::Key::D0
#define KEY_1               ::Graphics::Key::D1
#define KEY_2               ::Graphics::Key::D2
#define KEY_3               ::Graphics::Key::D3
#define KEY_4               ::Graphics::Key::D4
#define KEY_5               ::Graphics::Key::D5
#define KEY_6               ::Graphics::Key::D6
#define KEY_7               ::Graphics::Key::D7
#define KEY_8               ::Graphics::Key::D8
#define KEY_9               ::Graphics::Key::D9
#define KEY_SEMICOLON       ::Graphics::Key::Semicolon     /* ; */
#define KEY_EQUAL           ::Graphics::Key::Equal         /* = */
#define KEY_A               ::Graphics::Key::A
#define KEY_B               ::Graphics::Key::B
#define KEY_C               ::Graphics::Key::C
#define KEY_D               ::Graphics::Key::D
#define KEY_E               ::Graphics::Key::E
#define KEY_F               ::Graphics::Key::F
#define KEY_G               ::Graphics::Key::G
#define KEY_H               ::Graphics::Key::H
#define KEY_I               ::Graphics::Key::I
#define KEY_J               ::Graphics::Key::J
#define KEY_K               ::Graphics::Key::K
#define KEY_L               ::Graphics::Key::L
#define KEY_M               ::Graphics::Key::M
#define KEY_N               ::Graphics::Key::N
#define KEY_O               ::Graphics::Key::O
#define KEY_P               ::Graphics::Key::P
#define KEY_Q               ::Graphics::Key::Q
#define KEY_R               ::Graphics::Key::R
#define KEY_S               ::Graphics::Key::S
#define KEY               ::Graphics::Key::T
#define KEY_U               ::Graphics::Key::U
#define KEY_V               ::Graphics::Key::V
#define KEY_W               ::Graphics::Key::W
#define KEY_X               ::Graphics::Key::X
#define KEY_Y               ::Graphics::Key::Y
#define KEY_Z               ::Graphics::Key::Z
#define KEY_LEFT_BRACKET    ::Graphics::Key::LeftBracket   /* [ */
#define KEY_BACKSLASH       ::Graphics::Key::Backslash     /* \ */
#define KEY_RIGHT_BRACKET   ::Graphics::Key::RightBracket  /* ] */
#define KEY_GRAVE_ACCENT    ::Graphics::Key::GraveAccent   /* ` */
#define KEY_WORLD_1         ::Graphics::Key::World1        /* non-US #1 */
#define KEY_WORLD_2         ::Graphics::Key::World2        /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE          ::Graphics::Key::Escape
#define KEY_ENTER           ::Graphics::Key::Enter
#define KEYAB             ::Graphics::Key::Tab
#define KEY_BACKSPACE       ::Graphics::Key::Backspace
#define KEY_INSERT          ::Graphics::Key::Insert
#define KEY_DELETE          ::Graphics::Key::Delete
#define KEY_RIGHT           ::Graphics::Key::Right
#define KEY_LEFT            ::Graphics::Key::Left
#define KEY_DOWN            ::Graphics::Key::Down
#define KEY_UP              ::Graphics::Key::Up
#define KEY_PAGE_UP         ::Graphics::Key::PageUp
#define KEY_PAGE_DOWN       ::Graphics::Key::PageDown
#define KEY_HOME            ::Graphics::Key::Home
#define KEY_END             ::Graphics::Key::End
#define KEY_CAPS_LOCK       ::Graphics::Key::CapsLock
#define KEY_SCROLL_LOCK     ::Graphics::Key::ScrollLock
#define KEY_NUM_LOCK        ::Graphics::Key::NumLock
#define KEY_PRINT_SCREEN    ::Graphics::Key::PrintScreen
#define KEY_PAUSE           ::Graphics::Key::Pause
#define KEY_F1              ::Graphics::Key::F1
#define KEY_F2              ::Graphics::Key::F2
#define KEY_F3              ::Graphics::Key::F3
#define KEY_F4              ::Graphics::Key::F4
#define KEY_F5              ::Graphics::Key::F5
#define KEY_F6              ::Graphics::Key::F6
#define KEY_F7              ::Graphics::Key::F7
#define KEY_F8              ::Graphics::Key::F8
#define KEY_F9              ::Graphics::Key::F9
#define KEY_F10             ::Graphics::Key::F10
#define KEY_F11             ::Graphics::Key::F11
#define KEY_F12             ::Graphics::Key::F12
#define KEY_F13             ::Graphics::Key::F13
#define KEY_F14             ::Graphics::Key::F14
#define KEY_F15             ::Graphics::Key::F15
#define KEY_F16             ::Graphics::Key::F16
#define KEY_F17             ::Graphics::Key::F17
#define KEY_F18             ::Graphics::Key::F18
#define KEY_F19             ::Graphics::Key::F19
#define KEY_F20             ::Graphics::Key::F20
#define KEY_F21             ::Graphics::Key::F21
#define KEY_F22             ::Graphics::Key::F22
#define KEY_F23             ::Graphics::Key::F23
#define KEY_F24             ::Graphics::Key::F24
#define KEY_F25             ::Graphics::Key::F25

/* Keypad */
#define KEY_KP_0            ::Graphics::Key::KP0
#define KEY_KP_1            ::Graphics::Key::KP1
#define KEY_KP_2            ::Graphics::Key::KP2
#define KEY_KP_3            ::Graphics::Key::KP3
#define KEY_KP_4            ::Graphics::Key::KP4
#define KEY_KP_5            ::Graphics::Key::KP5
#define KEY_KP_6            ::Graphics::Key::KP6
#define KEY_KP_7            ::Graphics::Key::KP7
#define KEY_KP_8            ::Graphics::Key::KP8
#define KEY_KP_9            ::Graphics::Key::KP9
#define KEY_KP_DECIMAL      ::Graphics::Key::KPDecimal
#define KEY_KP_DIVIDE       ::Graphics::Key::KPDivide
#define KEY_KP_MULTIPLY     ::Graphics::Key::KPMultiply
#define KEY_KP_SUBTRACT     ::Graphics::Key::KPSubtract
#define KEY_KP_ADD          ::Graphics::Key::KPAdd
#define KEY_KP_ENTER        ::Graphics::Key::KPEnter
#define KEY_KP_EQUAL        ::Graphics::Key::KPEqual

#define KEY_LEFT_SHIFT      ::Graphics::Key::LeftShift
#define KEY_LEFT_CONTROL    ::Graphics::Key::LeftControl
#define KEY_LEFT_ALT        ::Graphics::Key::LeftAlt
#define KEY_LEFT_SUPER      ::Graphics::Key::LeftSuper
#define KEY_RIGHT_SHIFT     ::Graphics::Key::RightShift
#define KEY_RIGHT_CONTROL   ::Graphics::Key::RightControl
#define KEY_RIGHT_ALT       ::Graphics::Key::RightAlt
#define KEY_RIGHT_SUPER     ::Graphics::Key::RightSuper
#define KEY_MENU            ::Graphics::Key::Menu

// Mouse
#define MOUSE_BUTTON_LEFT    ::Graphics::Button::Left
#define MOUSE_BUTTON_RIGHT   ::Graphics::Button::Right
#define MOUSE_BUTTON_MIDDLE  ::Graphics::Button::Middle
