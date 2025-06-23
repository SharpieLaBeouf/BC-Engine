#pragma once

#include "PanelBase.h"

namespace BC
{

    class ProjectConfigPanel : public IEditorPanel
    {

    public:

        ProjectConfigPanel() = default;
        ~ProjectConfigPanel() = default;

        PanelType GetType() const override { return PanelType_ProjectConfig; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {

                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

    class SceneManagerConfigPanel : public IEditorPanel
    {

    public:

        SceneManagerConfigPanel() = default;
        ~SceneManagerConfigPanel() = default;

        PanelType GetType() const override { return PanelType_SceneManagerConfig; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {

                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

    class SceneConfigPanel : public IEditorPanel
    {

    public:

        SceneConfigPanel() = default;
        ~SceneConfigPanel() = default;

        PanelType GetType() const override { return PanelType_SceneConfig; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {

                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

}