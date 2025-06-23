#pragma once

#include "PanelBase.h"

namespace BC
{

    class SceneViewportPanel : public IEditorPanel
    {

    public:

        SceneViewportPanel() = default;
        ~SceneViewportPanel() = default;

        PanelType GetType() const override { return PanelType_SceneViewport; }

        void OnUpdate() override;
        void OnRenderGUI() override;

        bool IsFocused() const { return m_PanelFocused; }

    private:

        bool m_PanelFocused = false;
        bool m_ViewportResized = false;
        
        glm::uvec2 m_ViewportSize = { 1, 1 };

        friend class EditorLayer;

    };

}