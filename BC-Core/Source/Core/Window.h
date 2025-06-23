#pragma once

// Core Headers

// C++ Standard Library Headers
#include <memory>
#include <string>

// External Vendor Library Headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace BC
{

    enum class WindowedMode : uint8_t
    {
        Windowed,
        WindowedFullscreen,
        Fullscreen
    };

    struct WindowSpecification
    {
        std::string Name;
        WindowedMode Mode;
        uint32_t Width;
        uint32_t Height;

        uint32_t PosX;
        uint32_t PosY;
    };

    class Window
    {

    public: 
    
        // --------------------------------
        // Constructors & Operators
        // --------------------------------
        Window(const WindowSpecification& window_spec);
        ~Window();
    
        Window() = delete;
        Window(const Window& other) = delete;
        Window(Window&& other) = default;
        
        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) = default;
    
        // --------------------------------
        // General Methods
        // --------------------------------

        /// @brief Polls window events
        void OnUpdate() const;

        /// @brief Get a pointer to the native GLFW Window
        GLFWwindow* GetNativeWindow() const { return m_Window; }

        /// @brief Get the width of the current window
        uint32_t GetWidth()  const  { return m_Data->Width  == 0 ? 1 : m_Data->Width;   }
        
        /// @brief Get the height of the current window
        uint32_t GetHeight() const  { return m_Data->Height == 0 ? 1 : m_Data->Height;  }
        
        /// @brief Get the size (w,h) of the current window
        glm::uvec2 GetSize() const  { return { GetWidth(), GetHeight() };               }

        /// @brief Set the windowed mode of the window
        void SetWindowedMode(WindowedMode mode);

        /// @brief Link Callbacks
        void SetupCallbacks() const;

    private:

        /// @brief This will centre the window if windowed mode
        void CentreWindow();

        static void WindowPosCallback(GLFWwindow* window, int xpos, int ypos);
        static void WindowResizeCallback(GLFWwindow* window, int w, int h);
        static void WindowCloseCallback(GLFWwindow* window);

        static void WindowMouseCallback(GLFWwindow* window, int button, int action, int mods);
        static void WindowScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void WindowCursorCallback(GLFWwindow* window, double xpos, double ypos);
        static void WindowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void WindowCharCallback(GLFWwindow* window, unsigned int c);

    private:

        GLFWwindow* m_Window;
		std::unique_ptr<WindowSpecification> m_Data;
    };

}