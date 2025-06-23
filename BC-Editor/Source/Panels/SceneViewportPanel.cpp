#include "BC_PCH.h"
#include "SceneViewportPanel.h"

#include "BC-Editor.h"

namespace BC
{

    void SceneViewportPanel::OnUpdate()
    {
        // Toggle Mouse Hidden/Visible if Clicked Into SceneViewport
        if (IsFocused() && m_EditorLayer->GetSceneState() == SceneState_Play) 
        {
            glfwSetInputMode((GLFWwindow*)Application::GetWindow()->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        else 
        {
            glfwSetInputMode((GLFWwindow*)Application::GetWindow()->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        if (m_ViewportResized)
        {
            Application::Get()->ResizeScreenSpace(m_ViewportSize.x, m_ViewportSize.y);
            m_ViewportResized = false;
        }
    }

    void SceneViewportPanel::OnRenderGUI()
    {
        if (m_Active)
        {
            if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
            {
                ImVec2 panel_size = ImGui::GetContentRegionAvail();
                
                if (panel_size.x != m_ViewportSize.x || panel_size.y != m_ViewportSize.y)
                {
                    m_ViewportResized = true;
                    m_ViewportSize = { panel_size.x, panel_size.y };
                }

                // ImGui::Image
                // (
                //     (ImTextureID)m_EditorLayer->GetEditorCamera()->GetRenderTarget()->GetAttachments()[0].GetDescriptor(), 
                //     ImGui::GetContentRegionAvail(), 
                //     ImVec2{0, 1}, 
                //     ImVec2{1, 0}
                // );
            }
            ImGui::End();
        }
    }

}