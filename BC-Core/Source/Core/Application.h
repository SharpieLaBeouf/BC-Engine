#pragma once

// Core Headers
#include "Window.h"
#include "LayerStack.h"

#include "Graphics/Vulkan/VulkanCore.h"
#include "Project/Project.h"
#include "Jobs/JobSystem.h"
#include "Scripting/ScriptManager.h"

#include "Util/Platform.h"

// C++ Standard Library Headers
#include <string>
#include <memory>
#include <filesystem>
#include <functional>
#include <array>
#include <barrier>
#include <atomic>

// External Vendor Library Headers
#include <glm/glm.hpp>

int main(int argc, char** argv); 

#if defined(BC_PLATFORM_WINDOWS)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
#endif

namespace BC
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			return Args[index];
		}
	};

    struct BCApplicationSpecification
    {
		std::string Name = "BC Application";
		std::filesystem::path WorkingDirectory;
        ApplicationCommandLineArgs CommandLineArgs;

		glm::uvec2 WindowRes = { 1600, 900 };
		WindowedMode WindowMode = WindowedMode::Windowed;
		bool VSync = false;
		
    };

    class Application
    {
    
    public:

        Application() = delete;
		Application(BCApplicationSpecification& specification);
		virtual ~Application() = default;

		static Application* 	Get() 				{ return s_Instance ? s_Instance : nullptr; 						}
		static Window* 			GetWindow() 		{ return s_Instance ? s_Instance->m_Window.get() : nullptr; 		}
		static VulkanCore* 		GetVulkanCore() 	{ return s_Instance ? s_Instance->m_VulkanCore.get() : nullptr; 	}
		static Project* 		GetProject() 		{ return s_Instance ? s_Instance->m_Project.get() : nullptr; 		}
		static ScriptManager*	GetScriptManager() 	{ return s_Instance ? s_Instance->m_ScriptManager.get() : nullptr; 	}
		static JobSystem*		GetJobSystem()		{ return s_Instance ? s_Instance->m_JobSystem.get() : nullptr;		}

		static GUILayer* 		GetGUILayer() 		{ return s_Instance ? s_Instance->m_GUILayer : nullptr;				}

		const BCApplicationSpecification& GetSpecification() const { return m_Specification; }

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void SubmitToMainThread(const std::function<void()>& function);

		void ResizeScreenSpace(uint32_t width, uint32_t height);
		void ResizeSwapchain(const SwapchainSpecification& swapchain_spec);

		void SetMinimised(bool is_minimised) { m_Minimised = is_minimised; }

        void Close();

		void SetProject(std::unique_ptr<Project> project)
		{
			m_Project = std::move(project);
		}

		void SetScriptManager(std::unique_ptr<ScriptManager> script_manager)
		{
			m_ScriptManager = std::move(script_manager);
		}
	
    private:

		void Run();
		void ExecuteMainThreadQueue();

		void OnAnimationBlending();
		void OnAnimPhysTransformUpdate();

		void OnUpdate();
		void OnFixedUpdate();
		void OnLateUpdate();

		void RenderThreadWorker(uint32_t thread_core);

    private:

        BCApplicationSpecification m_Specification = {};

		std::atomic<bool> m_Running = true;
		std::atomic<bool> m_Minimised = false;

		float m_FixedUpdateTimer = 0.0f;
		
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<VulkanCore> m_VulkanCore;
		GUILayer* m_GUILayer;
		std::unique_ptr<LayerStack> m_LayerStack = std::make_unique<LayerStack>();

		/// @brief The overarching project for this Application instance
		std::unique_ptr<Project> m_Project = std::make_unique<Project>();
		
		std::unique_ptr<ScriptManager> m_ScriptManager = nullptr;

		// ----------------------------------
		// 			Multi-Threading
		// ----------------------------------

		std::unique_ptr<JobSystem> m_JobSystem = std::make_unique<JobSystem>();

		/// @brief Mutex to ensure thread safe push_back to the m_MainThreadQueue
		std::mutex m_MainThreadQueueMutex;

		/// @brief Functions that are sent to be processed by the main thread before script updates
		std::vector<std::function<void()>> m_MainThreadQueue;

		/// @brief The Render Thread, will be dispatched on Application creation and will hit the first barrier awaiting the Application::Run loop to then hit the barrier as well
		std::thread m_RenderThread;

		/// @brief Sync primitive to signal when both threads are at the start of the frame
		std::barrier<> m_FrameStartSync;
		
		/// @brief Sync primitive to signal when the render thread has finished rendering all commands, and the main thread is able to present to swapchain
		std::barrier<> m_FrameRenderFinishedSync;

		/// @brief Triple-buffered job counters for scene state capture.
		///
		/// Each counter tracks the capture of scene data for (frame_index + 2),
		/// used to prepare render commands ahead of time.
		std::vector<JobCounter> m_SceneSnapshotJobCounters = {};
		
		/// @brief Triple-buffered job counters for render command preparation.
		///
		/// Each counter tracks completion of render command preparation for 
		/// (frame_index + 1), which corresponds to the scene captured last frame
		/// and scheduled for rendering in the next frame.
		std::vector<JobCounter> m_PrepareRenderJobCounters = {};
    
	private:

		static Application* s_Instance;
		friend int Main_Entry(int argc, char** argv);

    };

	Application* CreateApplication(ApplicationCommandLineArgs args);
}