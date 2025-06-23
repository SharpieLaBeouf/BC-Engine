#include "BC_PCH.h"
#include "Window.h"

// Core Headers
#include "Application.h"
#include "Input.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{
    Window::Window(const WindowSpecification& window_spec) : 
        m_Data(std::make_unique<WindowSpecification>(window_spec))
    {
        BC_THROW(m_Data,   "Window::Window: Window Data Is Null.");

        BC_THROW(glfwInit() == GLFW_TRUE, "Window::Window: Could Not Initialise GLFW.");
        BC_THROW(glfwVulkanSupported() == GLFW_TRUE, "Window::Window: Vulkan not Supported.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(m_Data->Width, m_Data->Height, m_Data->Name.c_str(), nullptr, nullptr);
        BC_THROW(m_Window, "Window::Window: Could Not Create Window.");
        
        glfwSetWindowUserPointer(m_Window, this);

        CentreWindow(); // Center if Windowed

        OnUpdate(); // Poll events on init

        BC_CORE_TRACE("Window::Window: GLFW Window Created Successfully - X: {}, Y: {}.", m_Data->Width, m_Data->Height);
    }

    Window::~Window()
    {
        m_Data.reset();
        m_Data = nullptr;

		glfwDestroyWindow(m_Window);
        m_Window = nullptr;

		glfwTerminate();
    }

    void Window::OnUpdate() const
    {
		glfwPollEvents();
    }

    void Window::SetWindowedMode(WindowedMode mode)
    {
        if (m_Data->Mode == mode)
            return;

        GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);

        switch (mode)
        {
            case WindowedMode::Windowed:
            {
                // Revert to standard windowed with decorations
                glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_TRUE);
                glfwSetWindowMonitor(m_Window, nullptr,
                                    m_Data->PosX, m_Data->PosY,
                                    m_Data->Width, m_Data->Height, 0);

                CentreWindow();
                break;
            }

            case WindowedMode::WindowedFullscreen:
            {
                // Borderless fullscreen window
                glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_FALSE);
                glfwSetWindowMonitor(m_Window, nullptr, 0, 0,
                                    video_mode->width, video_mode->height, 0);
                break;
            }

            case WindowedMode::Fullscreen:
            {
                // Exclusive fullscreen mode
                glfwSetWindowMonitor(m_Window, primary_monitor, 0, 0,
                                    video_mode->width, video_mode->height,
                                    video_mode->refreshRate);
                break;
            }
        }

        m_Data->Mode = mode;
    }

    void Window::SetupCallbacks() const
    {
        glfwSetWindowPosCallback        (m_Window, WindowPosCallback);
        glfwSetFramebufferSizeCallback  (m_Window, WindowResizeCallback);
        glfwSetWindowCloseCallback      (m_Window, WindowCloseCallback);

        glfwSetMouseButtonCallback      (m_Window, WindowMouseCallback);
        glfwSetCursorPosCallback        (m_Window, WindowCursorCallback);
        glfwSetScrollCallback           (m_Window, WindowScrollCallback);
        glfwSetKeyCallback              (m_Window, WindowKeyCallback);
        glfwSetCharCallback             (m_Window, WindowCharCallback);
    }

    void Window::CentreWindow()
    {
        if (m_Data->Mode != WindowedMode::Windowed)
            return;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        int work_x, work_y, work_width, work_height;
        glfwGetMonitorWorkarea(monitor, &work_x, &work_y, &work_width, &work_height);

        m_Data->PosX = work_x + (work_width - m_Data->Width) / 2;
        m_Data->PosY = work_y + (work_height - m_Data->Height) / 2;

        glfwSetWindowPos(m_Window, m_Data->PosX, m_Data->PosY);
        BC_CORE_TRACE("Window::Window: Window centered at X: {}, Y: {}.", m_Data->PosX, m_Data->PosY);
    }

    void Window::WindowPosCallback(GLFWwindow *window, int xpos, int ypos)
    {
        Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (window_instance && window_instance->m_Data)
        {
            window_instance->m_Data->PosX = xpos;
            window_instance->m_Data->PosY = ypos;
        }
    }

    void Window::WindowResizeCallback(GLFWwindow *window, int w, int h)
    {
        Window* window_instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (window_instance && window_instance->m_Data)
        {
            window_instance->m_Data->Width = w;
            window_instance->m_Data->Height = h;
        }
        
        Application::Get()->SetMinimised(w == 0 || h == 0);

        auto physical_device = Application::GetVulkanCore()->GetPhysicalDevice();
        auto surface = Application::GetVulkanCore()->GetSurface();
        auto capabilities = Swapchain::GetSwapchainSupport(physical_device, surface);
        Swapchain::s_MinImageCount = std::max<uint32_t>(capabilities.Capabilities.minImageCount, 2);
        BC_THROW(Swapchain::s_MinImageCount >= 2, "Window::WindowResizeCallback: Min Image Count Must Be Atleast 2.");
        SwapchainSpecification swapchain_spec = 
        {
            .ImageCount = static_cast<uint8_t>(Swapchain::s_MinImageCount + 1),
            .ImageFormat = Swapchain::ChooseSwapchainFormat(capabilities.Formats),
            .PresentMode = Swapchain::ChooseSwapchainPresentMode(capabilities.PresentModes),
            .Extent = Swapchain::ChooseSwapchainExtent(capabilities.Capabilities)
        };

        Application::Get()->ResizeSwapchain(swapchain_spec);
    }
    
    void Window::WindowCloseCallback(GLFWwindow* window)
    {
        Application::Get()->Close();
    }

    void Window::WindowMouseCallback(GLFWwindow *window, int button, int action, int mods)
    {
        Input::InputMouseCallback(window, button, action, mods);
    }

    void Window::WindowScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
    {
        Input::InputScrollCallback(window, xoffset, yoffset);
    }

    void Window::WindowCursorCallback(GLFWwindow *window, double xpos, double ypos)
    {
        Input::InputCursorCallback(window, xpos, ypos);
    }

    void Window::WindowKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        Input::InputKeyCallback(window, key, scancode, action, mods);
    }

    void Window::WindowCharCallback(GLFWwindow *window, unsigned int c)
    {
        Input::InputCharCallback(window, c);
    }
}