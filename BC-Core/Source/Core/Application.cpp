#include "BC_PCH.h"
#include "Application.h"

// Core Headers
#include "Time.h"
#include "Input.h"

#include "Graphics/Renderer/SceneRenderer.h"

#include "Physics/Physics.h"

#include "Util/ThreadUtil.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{
    Application* Application::s_Instance = nullptr;

    Application::Application(BCApplicationSpecification &specification) : 
        m_Specification(std::move(specification)),
        m_FrameStartSync(2),
        m_FrameRenderFinishedSync(2)
    {
        BC_CATCH_BEGIN();
        
        BC_CORE_TRACE("Application::Application: Initialising Bat and Cat Engine.");
            
        s_Instance = this;

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_JobSystem->Init();

        Time::Init();
        Input::Init();
        Physics::Init();

        WindowSpecification window_info = 
        {
            .Name = m_Specification.Name,
            .Mode = m_Specification.WindowMode,
            .Width = m_Specification.WindowRes.x,
            .Height = m_Specification.WindowRes.y,
            .PosX = 0,
            .PosY = 0
        };

        m_Window = std::make_unique<Window>(window_info);
        
        m_VulkanCore = std::make_unique<VulkanCore>();
        m_VulkanCore->Init(m_Specification.Name.c_str(), m_Window->GetNativeWindow());

        m_SceneSnapshotJobCounters.resize(Swapchain::s_MinImageCount + 1);
        m_PrepareRenderJobCounters.resize(Swapchain::s_MinImageCount + 1);
        
        m_GUILayer = new GUILayer();
        PushOverlay(m_GUILayer);

        m_Window->SetupCallbacks();
        SceneRenderer::Init();
    
        BC_CATCH_END_FUNC([&]() { Close(); });
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack->PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack->PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::Run()
    {
        BC_CATCH_BEGIN();

        m_RenderThread = std::thread([&]() { RenderThreadWorker(1); });

        while (m_Running)
        {
            {
                BC_PROFILE_SCOPE("Main Thread: Waiting For Render Thread to Finish Previous Frame.");
                m_FrameStartSync.arrive_and_wait();
            }

            BC_PROFILE_SCOPE("Application::Run: Main Loop");

            m_JobSystem->BeginFrameProfile();
            Profiler::Get().NewFrame();
            Time::UpdateTime();

            auto current_frame_index = m_VulkanCore->GetFrameIndex();

            JobCounter animation_blending_job_counter = {};
            JobCounter transform_update_job_counter = {}; // To be used after above two counters finished

            m_JobSystem->SubmitJob
            (
                "Animation Blending", 
                [&]() { OnAnimationBlending(); }, 
                &animation_blending_job_counter,
                JobPriority::Medium,
                false
            ); // Perform animation blending

            OnUpdate();
            OnFixedUpdate();
            OnLateUpdate();

            // Execute Main Thread Function Queue
            ExecuteMainThreadQueue();

            // Scene's have been updated, now we can wait for Animation and
            // Physics prior to updating the ECS with those changes too
            animation_blending_job_counter.Wait();

            m_JobSystem->SubmitJob
            (
                "ECS Transform Update - Animation & Physics",
                [&]() { OnAnimPhysTransformUpdate(); },
                &transform_update_job_counter, 
                JobPriority::High,
                false
            ); // Update ECS with all transform changes from Animation Blending and Physics Simulation

            // 5. SwapChain Rendering
            if (!m_Minimised)
            {
                m_VulkanCore->BeginFrame();

                {
                    BC_PROFILE_SCOPE("Application::Run: 5. GUI Update Loop");
                    m_GUILayer->Begin();

                    for (Layer* layer : *m_LayerStack) 
                    {
                        layer->OnRenderGUI();
                    }

                    m_VulkanCore->BeginSwapchainRenderPass();
                    m_GUILayer->End();
                    m_VulkanCore->EndSwapchainRenderPass();
                }

                // Sync with Render Thread once off screen rendering completed
                {
                    BC_PROFILE_SCOPE("Main Thread: Waiting For Render Thread to Finish Rendering.");
                    m_FrameRenderFinishedSync.arrive_and_wait();
                }

                // Submit ImGui to Swapchain Image, Present Swapchain Image, Wrapped Increment Frame Index
                m_VulkanCore->EndFrame();
            }
            else
            {
                // If minimised, still need sync point on both threads to ensure no dead locks
                BC_PROFILE_SCOPE("Main Thread: Waiting For Render Thread to Finish Rendering.");
                m_FrameRenderFinishedSync.arrive_and_wait();
            }

            // End Frame, Poll Events
            {
                BC_PROFILE_SCOPE("Application::Run: 6. Update Window");
                Input::EndFrame();
                m_Window->OnUpdate();
            }

            transform_update_job_counter.Wait();

            m_JobSystem->FinishJobs();
            m_JobSystem->EndFrameProfile();
        }

        BC_CATCH_END_FUNC([&]() { Close(); });

        m_FrameStartSync.arrive_and_drop();
        m_FrameRenderFinishedSync.arrive_and_drop();

        if (m_RenderThread.joinable())
            m_RenderThread.join();
        
        Physics::Shutdown();
        Input::Shutdown();
        Time::Shutdown();
        
        m_JobSystem->Shutdown();
        m_JobSystem.reset();

        vkDeviceWaitIdle(m_VulkanCore->GetLogicalDevice());

        SceneRenderer::Shutdown();
    }

    void Application::RenderThreadWorker(uint32_t thread_core)
    {
        BC_CATCH_BEGIN();

        BC_THROW(thread_core >= 0 && thread_core < std::thread::hardware_concurrency(), "Application::RenderThreadWorker: Invalid Thread Core Index.");

        Util::SetThreadCoreAffinity(thread_core);

        while(m_Running)
        {
            {
                BC_PROFILE_SCOPE("Render Thread: Waiting For Main Thread to Finish Previous Frame.");
                m_FrameStartSync.arrive_and_wait();
            }

            BC_PROFILE_SCOPE("Application::RenderThreadWorker: Render Loop");

            auto current_frame_index = m_VulkanCore->GetFrameIndex();

            // ----------------------------
            //     Render Thread Flow
            //
            // 1. Kick Job to SnapshotScene for N + 2
            //      a. Collect override cameras from attached layers if any
            //      b. Snapshot scene state including lighting, geometry, etc.
            // 2. Kick Job to RecordCommandBuffers for N + 1
            //      a. Wait for SnapshotScene from last frame to complete
            //      b. Record CommandBuffers
            // 3. Wait for m_PrepareRenderJobCounters on frame N
            // 4. Submit Command Buffers once job complete
            // ----------------------------

            // Kick Job to Snapshot Current Scene State for Rendering in Frame N + 2
            m_JobSystem->SubmitJob
            (
                "Render Prep: Snapshot Scene State", 
                [&]() 
                { 
                    std::vector<CameraContext> camera_overrides = {};
                    for (Layer* layer : *m_LayerStack)
                    {
                        camera_overrides = layer->GetCameraOverrides();
                        if (!camera_overrides.empty())
                            break;
                    }
                    SceneRenderer::SnapshotScene((current_frame_index + 2) % m_VulkanCore->GetSwapchain().GetImageCount(), camera_overrides); 
                }, 
                &m_SceneSnapshotJobCounters[(current_frame_index + 2) % m_VulkanCore->GetSwapchain().GetImageCount()], 
                JobPriority::High, // Needs to be done quick before update functions start modifying the scene state!
                false
            );

            // Kick Job to Record Command Buffers for frame N + 1
            m_JobSystem->SubmitJob
            (
                "Render Prep: Record Command Buffers for Frame N + 1", 
                [&]() 
                { 
                    m_SceneSnapshotJobCounters[(current_frame_index + 1) % m_VulkanCore->GetSwapchain().GetImageCount()].Wait();
                    SceneRenderer::RecordCommandBuffers((current_frame_index + 1) % m_VulkanCore->GetSwapchain().GetImageCount()); 
                }, 
                &m_PrepareRenderJobCounters[(current_frame_index + 1) % m_VulkanCore->GetSwapchain().GetImageCount()], 
                JobPriority::Medium, 
                true
            );
            
            // Wait for this frames OnRecordCommandBuffers job to complete (invoked from last frame SEE ABOVE)
            m_PrepareRenderJobCounters[m_VulkanCore->GetFrameIndex()].Wait();

            // Render
            // 1. Submit Render Command Buffers for frame N
            // 2. Wait for GPU to Complete Work for frame N

            {
                BC_PROFILE_SCOPE("Render Thread: Waiting For Main Thread to Finish Updates.");
                m_FrameRenderFinishedSync.arrive_and_wait();
            }

            // Cleanup
            // 1. Clean Current Frame Data? E.g., LightEnvironment,
            //    GeometryEnvironment, etc.?

        }
        
        BC_CATCH_END_FUNC([&]() { Close(); });

        return;
    }

    void Application::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    void Application::ExecuteMainThreadQueue()
    {
        BC_PROFILE_SCOPE("Application::ExecuteMainThreadQueue: Execute Main Thread Queue");
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }

    void Application::OnAnimationBlending()
    {

    }

    void Application::OnAnimPhysTransformUpdate()
    {
        // 1. Update Physics Transforms
        auto physics_system = m_Project->GetSceneManager()->GetPhysicsSystem();
        if (physics_system)
            physics_system->OnTransformUpdate();

        // 2. Update Animation Transforms
        // TODO: Implement
    }

    void Application::OnUpdate()
    {
        BC_PROFILE_SCOPE("Application::OnUpdate: On Update Loop");
        for (Layer* layer : *m_LayerStack) 
        {
            layer->OnUpdate();
        }
        
        if (m_Project && m_Project->GetSceneManager())
            m_Project->GetSceneManager()->OnUpdate();
    }

    void Application::OnFixedUpdate()
    {
        BC_PROFILE_SCOPE("Application::OnFixedUpdate: On Fixed Update Loop");
        m_FixedUpdateTimer += Time::GetDeltaTime();
        while (m_FixedUpdateTimer >= Time::GetUnscaledFixedDeltaTime()) 
        {
            for (Layer* layer : *m_LayerStack) 
            {
                layer->OnFixedUpdate();
            }
        
            if (m_Project && m_Project->GetSceneManager())
                m_Project->GetSceneManager()->OnFixedUpdate();

            m_FixedUpdateTimer -= Time::GetUnscaledFixedDeltaTime();
        }
    }

    void Application::OnLateUpdate()
    {
        BC_PROFILE_SCOPE("Application::OnLateUpdate: On Late Update Loop");
        for (Layer* layer : *m_LayerStack) 
        {
            layer->OnLateUpdate();
        }

        if (m_Project && m_Project->GetSceneManager())
            m_Project->GetSceneManager()->OnLateUpdate();
    }

    void Application::ResizeScreenSpace(uint32_t width, uint32_t height)
    {
        // 1. Update Layers Attached to Application for any Vulkan resources
        //    relying on screen space dimensions
        for (auto layer : *m_LayerStack)
        {
            if (!layer)
                continue;

            layer->ResizeScreenSpace(width, height);
        }

        // 2. Update Static Scene Renderer for any Vulkan resources relying on
        //    screen space dimensions 
        // SceneRenderer::ResizeScreenSpace(width, height);

        // 3. Update Vulkan Core with Screen Space
        m_VulkanCore->ResizeScreenSpace(width, height);
        
        BC_CORE_TRACE("Application::ResizeScreenSpace: Resized ScreenSpace Successfully - X:{}, Y:{}.", width, height);
    }

    void Application::ResizeSwapchain(const SwapchainSpecification& swapchain_spec)
    {
        if (m_Minimised) // Don't recreate on minimise
            return;

        // 1. Update Layers Attached to Application for any Vulkan resources
        //    relying on swapchain
        for (auto layer : *m_LayerStack)
        {
            if (!layer)
                continue;

            layer->ResizeSwapchain(swapchain_spec);
        }

        // 2. Update Static Scene Renderer for any Vulkan resources relying on
        //    swapchain
        //SceneRenderer::ResizeSwapchain(width, height);

        // 3. Update Vulkan Swapchain
        m_VulkanCore->ResizeSwapchain(swapchain_spec);
        
        BC_CORE_TRACE("Application::ResizeSwapchain: Resized Swapchain Successfully - X:{}, Y:{}.", swapchain_spec.Extent.width, swapchain_spec.Extent.height);
    }

}

/*

    // Main Thread
    while (m_Running)
    {
        {
            BC_PROFILE_SCOPE_ACCUMULATIVE("Wait for Render Thread");
            sync_start.arrive_and_wait(); // std::barrier(2) -> wait for render thread to begin
        }

        std::vector<CameraContext> override_cameras
        for (Layer* layer : *m_LayerStack) 
        {
            override_cameras = layer->CheckOverrideCameras();
            if (!override_cameras.empty())
            {
                break;
            }
        }

        // Kick CaptureScene, Animation, and Physics Jobs
        // Capture scene for N+2 scene, if camera overrides parameter has a vector that is not emppty, 
        // it will scrap cameras from the world, and only render the camera contexts passed through the vector
        m_JobSystem.SubmitJob("Capture Scene N + 2", OnSnapshotScene(m_FrameIndex + 2, override_cameras), &m_SceneSnapshotJobCounters[m_FrameIndex + 2], true); 

        JobCounter animation_blending_job_counter = {};
        m_JobSystem.SubmitJob("Animation Blending", OnAnimationBlending(), &animation_blending_job_counter, false); // Perform animation blending

        JobCounter physics_simulate_job_counter = {};
        m_JobSystem.SubmitJob("Physics Simulation", OnPhysicsSimulation(), &physics_simulate_job_counter, false); // Perform physics simulations

        // Call Editor/Runtime UI commands for ImGui prior to off-screen render finish sync point
        ImGui::NewFrame();
        ImGui::Image(scene_frame[frame_index].final_image_descriptor, ...);
        RenderEditorUI(); // Other ImGui widgets
        ImGui::EndFrame();

        // Update scripts on main thread
        m_SceneSnapshotJobCounters[m_FrameIndex + 2].Wait(); // Capture scene first, then process changes to scene after captured

        OnScriptUpdate();
        OnScriptLateUpdate();
        OnScriptFixedUpdate();

        // Wait Physics & Animation Job's
        animation_blending_job_counter.Wait();
        physics_simulate_job_counter.Wait();

        JobCounter animation_physics_transform_update_job_counter = {};
        m_JobSystem.SubmitJob("Animation & Physics Transform Update into ECS", OnAnimPhysTransformUpdate(), &animation_physics_transform_update_job_counter, false);

        // Wait for render thread to finish rendering N
        {
            BC_PROFILE_SCOPE_ACCUMULATIVE("Wait for Render Thread");
            sync_render_finished.arrive_and_wait();
        }

        // Render ImGui Commands
        ImGui::Render();

        // Submit ImGui render pass to swapchain
        SubmitImGuiToSwapchain(ImGui::GetDrawData());

        // Present
        PresentSwapchainImage();

        // Wait for transform updates to occur before continuing to next frame which will start capturing and recording command buffers
        animation_physics_transform_update_job_counter.Wait();

        // 12. Wait for Job System to finish non-persistent jobs (only last in existing frame and should be completed prior to continuing to next frame)
        m_JobSystem.FinishNonPersistentJobs();

        frame_index = (frame_index + 1) % NUM_FRAMES_IN_FLIGHT;
    }

    // Render Thread
    while (m_Running)
    {
        {
            BC_PROFILE_SCOPE_ACCUMULATIVE("Wait for Main Thread");
            sync_start.arrive_and_wait(); // std::barrier(2) -> wait for main thread to begin
        }

        m_JobSystem.SubmitJob("Record Command Buffers N + 1", OnRecordCommandBuffers(m_FrameIndex + 1), &m_RenderCmdBufferJobCounters[m_FrameIndex + 1], true); // Record command buffers for captured scene at N+1
        
        // 1. Collect and submit command buffers from SceneFrameInFlight[N]
        SubmitRenderPass(scene_frame[frame_index]);

        // 2. Wait for GPU to complete rendering work
        WaitForGPUFenceOrTimeline(scene_frame[frame_index]);

        // Sync point that render thread has finished rendering N
        {
            BC_PROFILE_SCOPE_ACCUMULATIVE("Wait for Main Thread");
            sync_render_finished.arrive_and_wait();
        }

        // 3. (Optional) Cleanup transient resources
        CleanUpTransientResources(scene_frame[frame_index]);

        // Wait for command buffers to be filled for rendering next frame N+1
        m_RenderCmdBufferJobCounters[m_FrameIndex + 1].Wait();
    }

*/