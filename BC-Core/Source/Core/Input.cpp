#include "BC_PCH.h"
#include "Input.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

namespace BC
{
    Input* Input::s_Instance = nullptr;

	Input::Input() : 
        m_MouseX(0.0), 
        m_MouseY(0.0), 
        m_ScrollX(0.0), 
        m_ScrollY(0.0)
    {
		for (int i = 0; i < MAX_KEYS; i++)
        {
			for (int x = 0; x < 2; x++)
            {
				m_Keys[i][x] = false;
            }
        }

		for (int i = 0; i < MAX_BUTTONS; i++)
        {
			for (int x = 0; x < 2; x++)
            {
				m_Buttons[i][x] = false;
            }
        }
	}

	void Input::Init()
	{
		BC_ASSERT(!s_Instance, "Input Already Initialised!");
		s_Instance = new Input();
	}

	void Input::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

    bool Input::GetKey(KeyCode key_code)
    {
		BC_ASSERT(s_Instance, "Input::GetKey: Input Not Initialised!");
        if (!s_Instance)
            return false;

		return s_Instance->m_Keys[key_code][0];
    }

    bool Input::GetKeyDown(KeyCode key_code)
    {
		BC_ASSERT(s_Instance, "Input::GetKeyDown: Input Not Initialised!");
        if (!s_Instance)
            return false;

        return s_Instance->m_Keys[key_code][0] == true && s_Instance->m_Keys[key_code][1] == true;
    }
    bool Input::GetKeyUp(KeyCode key_code)
    {
		BC_ASSERT(s_Instance, "Input::GetKeyUp: Input Not Initialised!");
        if (!s_Instance)
            return false;

        return s_Instance->m_Keys[key_code][0] == false && s_Instance->m_Keys[key_code][1] == true;
    }

    bool Input::GetMouseButton(MouseButtonCode button_code)
    {
		BC_ASSERT(s_Instance, "Input::GetMouseButton: Input Not Initialised!");
        if (!s_Instance)
            return false;

        return s_Instance->m_Buttons[button_code][0];
    }

    bool Input::GetMouseButtonDown(MouseButtonCode button_code)
    {
		BC_ASSERT(s_Instance, "Input::GetMouseButtonDown: Input Not Initialised!");
        if (!s_Instance)
            return false;

        return s_Instance->m_Buttons[button_code][0] == true && s_Instance->m_Buttons[button_code][1] == true;
    }

    bool Input::GetMouseButtonUp(MouseButtonCode button_code)
    {
		BC_ASSERT(s_Instance, "Input::GetMouseButtonUp: Input Not Initialised!");
        if (!s_Instance)
            return false;

        return s_Instance->m_Buttons[button_code][0] == false && s_Instance->m_Buttons[button_code][1] == true;
    }

    float Input::GetMouseX()
    {
		BC_ASSERT(s_Instance, "Input::GetMouseX: Input Not Initialised!");
        if (!s_Instance)
            return 0.0f;

        return static_cast<float>(s_Instance->m_MouseX);
    }

    float Input::GetMouseY()
    {
		BC_ASSERT(s_Instance, "Input::GetMouseY: Input Not Initialised!");
        if (!s_Instance)
            return 0.0f;

        return static_cast<float>(s_Instance->m_MouseY);
    }

     glm::vec2 Input::GetMousePosition() 
     { 
		BC_ASSERT(s_Instance, "Input::GetMousePosition: Input Not Initialised!");
        if (!s_Instance)
            return glm::vec2(0.0f);

        return { static_cast<float>(s_Instance->m_MouseX), static_cast<float>(s_Instance->m_MouseY) }; 
    }

    float Input::GetScrollX()
    {
		BC_ASSERT(s_Instance, "Input::GetScrollX: Input Not Initialised!");
        if (!s_Instance)
            return 0.0f;

        return static_cast<float>(s_Instance->m_ScrollX);
    }

    float Input::GetScrollY()
    {
		BC_ASSERT(s_Instance, "Input::GetScrollY: Input Not Initialised!");
        if (!s_Instance)
            return 0.0f;

        return static_cast<float>(s_Instance->m_ScrollY);
    }

    void Input::EndFrame()
    {
		BC_ASSERT(s_Instance, "Input::GetScrollY: Input Not Initialised!");
        if (!s_Instance)
            return;

		// Clear key state changes
		for (int i = 0; i < MAX_KEYS; ++i)
			s_Instance->m_Keys[i][1] = false;

		// Clear button state changes
		for (int i = 0; i < MAX_BUTTONS; ++i)
			s_Instance->m_Buttons[i][1] = false;

		s_Instance->m_ScrollX = 0.0;
		s_Instance->m_ScrollY = 0.0;
    }

    void Input::InputMouseCallback(GLFWwindow* window, int button, int action, int mods)
    {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

		BC_ASSERT(s_Instance, "Input::InputMouseCallback: Input Not Initialised!");
        if (!s_Instance)
            return;

		if (action == GLFW_PRESS)
		{
			s_Instance->m_Buttons[button][0] = true;
			s_Instance->m_Buttons[button][1] = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_Instance->m_Buttons[button][0] = false;
			s_Instance->m_Buttons[button][1] = true;
		}
    }

    void Input::InputScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

		BC_ASSERT(s_Instance, "Input::InputScrollCallback: Input Not Initialised!");
        if (!s_Instance)
            return;

		s_Instance->m_ScrollX += xoffset;
		s_Instance->m_ScrollY += yoffset;
    }

    void Input::InputCursorCallback(GLFWwindow* window, double xpos, double ypos)
    {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

		BC_ASSERT(s_Instance, "Input::InputCursorCallback: Input Not Initialised!");
        if (!s_Instance)
            return;

		s_Instance->m_MouseX = xpos;
		s_Instance->m_MouseY = ypos;

    }

    void Input::InputKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

		BC_ASSERT(s_Instance, "Input::InputKeyCallback: Input Not Initialised!");
        if (!s_Instance)
            return;

		if (action == GLFW_PRESS)
		{
			s_Instance->m_Keys[key][0] = true;
			s_Instance->m_Keys[key][1] = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_Instance->m_Keys[key][0] = false;
			s_Instance->m_Keys[key][1] = true;
		}
    }

    void Input::InputCharCallback(GLFWwindow *window, unsigned int c)
    {
        ImGui_ImplGlfw_CharCallback(window, c);
    }
}