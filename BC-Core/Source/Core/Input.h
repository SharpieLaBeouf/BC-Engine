#pragma once

// Core Headers
#include "Keycode.h"

// C++ Standard Library Headers
#include <cstdint>

// External Vendor Library Headers
#include <glm/glm.hpp>

struct GLFWwindow;

namespace BC
{
    class Input 
    {
        static const uint32_t MAX_KEYS = 1024; 
        static const uint32_t MAX_BUTTONS = 32; 

    public:

        static void Init();
        static void Shutdown();

		static bool GetKey(KeyCode key_code);
		static bool GetKeyDown(KeyCode key_code);
		static bool GetKeyUp(KeyCode key_code);
 
		static bool GetMouseButton(MouseButtonCode button_code);
		static bool GetMouseButtonDown(MouseButtonCode button_code);
		static bool GetMouseButtonUp(MouseButtonCode button_code);

		static float GetMouseX();
		static float GetMouseY();

		static glm::vec2 GetMousePosition();

		static float GetScrollX();
		static float GetScrollY();

		static void EndFrame();

    private:

		bool s_MouseHidden = true;

		bool     m_Keys[MAX_KEYS][2];
		bool     m_Buttons[MAX_BUTTONS][2];

		double   m_MouseX, m_MouseY;
		double   m_ScrollX, m_ScrollY;

		static Input* s_Instance;

    private:

        Input();

		// Delete copy assignment and move assignment constructors
        Input(const Input& other) = delete;
        Input(Input&& other) = delete;

		// Delete copy assignment and move assignment operators
        Input& operator=(const Input& other) = delete;
        Input& operator=(Input&& other) = delete;

        friend class Window;

		static void InputMouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void InputScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void InputCursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void InputKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void InputCharCallback(GLFWwindow* window, unsigned int c);

    };
}