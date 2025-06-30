#pragma once
#include "BC.h"

#include "Panels/PanelBase.h"
#include "Panels/AnimatorPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/HumanoidConfigPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ProfilerPanel.h"
#include "Panels/SceneConfigPanels.h"
#include "Panels/SceneViewportPanel.h"
#include "Panels/StatisticsPanel.h"

#include "Camera/EditorCamera.h"

#include <vector>

#include <cstdint>

namespace BC
{

    using SceneState = uint8_t;
    enum : SceneState
    {
        SceneState_Play,
        SceneState_Edit,
        SceneState_Sim
    };

    class EditorLayer : public Layer
    {

    public:

        EditorLayer();
        ~EditorLayer();

        void ResizeScreenSpace(uint32_t width, uint32_t height) override;
        void ResizeSwapchain(const SwapchainSpecification& swapchain_spec) override;

        void OnAttach() override;
        void OnDetach() override;

        void OnUpdate() override;
        void OnFixedUpdate() override;
        void OnLateUpdate() override;
        
        std::vector<CameraContext> GetCameraOverrides() override;

        void OnRenderGUI() override;

        void SetSceneState(SceneState state) { m_SceneState = state; }
        SceneState GetSceneState() const { return m_SceneState; }

        EditorCamera* GetEditorCamera() { return &m_Camera; }

    private:

        void OpenScene(bool additive, const std::filesystem::path& scene_file_path = "");

        void OnStartRuntime();
        void OnStartSimulation();

        void OnStep();

        void OnStopSceneManager();

        void DrawTopBar();
        void DrawDockspace();

    private:

        EditorCamera m_Camera;

        SceneState m_SceneState = SceneState_Edit;

        std::unordered_map<PanelType, std::unique_ptr<IEditorPanel>> m_Panels = {};

        std::unique_ptr<SceneManager> m_CachedSceneManager = nullptr;

        std::shared_ptr<Texture2D> m_PlayButton = nullptr;
        std::shared_ptr<Texture2D> m_PauseButton = nullptr;
        std::shared_ptr<Texture2D> m_StopButton = nullptr;
        std::shared_ptr<Texture2D> m_SimButton = nullptr;
        std::shared_ptr<Texture2D> m_StepButton = nullptr;

    public:

        template<typename TPanelType>
        TPanelType* GetPanel()
        {
            if constexpr        (std::is_same_v<TPanelType, AnimatorPanel>)
            {
                if (!m_Panels.contains(PanelType_AnimatorNodeGraph)) return nullptr;
                return reinterpret_cast<AnimatorPanel*>(m_Panels[PanelType_AnimatorNodeGraph].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, AssetRegistryPanel>)
            {
                if (!m_Panels.contains(PanelType_AssetRegistry)) return nullptr;
                return reinterpret_cast<AssetRegistryPanel*>(m_Panels[PanelType_AssetRegistry].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, ConsolePanel>)
            {
                if (!m_Panels.contains(PanelType_Console)) return nullptr;
                return reinterpret_cast<ConsolePanel*>(m_Panels[PanelType_Console].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, ContentBrowserPanel>)
            {
                if (!m_Panels.contains(PanelType_ContentBrowser)) return nullptr;
                return reinterpret_cast<ContentBrowserPanel*>(m_Panels[PanelType_ContentBrowser].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, HierarchyPanel>)
            {
                if (!m_Panels.contains(PanelType_Hierarchy)) return nullptr;
                return reinterpret_cast<HierarchyPanel*>(m_Panels[PanelType_Hierarchy].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, HumanoidConfigPanel>)
            {
                if (!m_Panels.contains(PanelType_HumanoidConfig)) return nullptr;
                return reinterpret_cast<HumanoidConfigPanel*>(m_Panels[PanelType_HumanoidConfig].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, InspectorPanel>)
            {
                if (!m_Panels.contains(PanelType_Inspector)) return nullptr;
                return reinterpret_cast<InspectorPanel*>(m_Panels[PanelType_Inspector].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, ProfilerPanel>)
            {
                if (!m_Panels.contains(PanelType_Profiler)) return nullptr;
                return reinterpret_cast<ProfilerPanel*>(m_Panels[PanelType_Profiler].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, SceneManagerConfigPanel>)
            {
                if (!m_Panels.contains(PanelType_SceneManagerConfig)) return nullptr;
                return reinterpret_cast<SceneManagerConfigPanel*>(m_Panels[PanelType_SceneManagerConfig].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, SceneConfigPanel>)
            {
                if (!m_Panels.contains(PanelType_SceneConfig)) return nullptr;
                return reinterpret_cast<SceneConfigPanel*>(m_Panels[PanelType_SceneConfig].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, SceneViewportPanel>)
            {
                if (!m_Panels.contains(PanelType_SceneViewport)) return nullptr;
                return reinterpret_cast<SceneViewportPanel*>(m_Panels[PanelType_SceneViewport].get());
            }
            else if constexpr   (std::is_same_v<TPanelType, StatisticsPanel>)
            {
                if (!m_Panels.contains(PanelType_Statistics)) return nullptr;
                return reinterpret_cast<StatisticsPanel*>(m_Panels[PanelType_Statistics].get());
            }
            return nullptr;
        }

    };

}